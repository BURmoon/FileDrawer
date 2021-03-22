/*
 * 上传文件相关的后台CGI程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include "deal_mysql.h"
#include "fcgi_stdio.h"
#include "cfg.h"
#include "util_cgi.h"
#include "make_log.h"

#define UPLOAD_LOG_MODULE "cgi"
#define UPLOAD_LOG_PROC   "upload"

//mysql 数据库配置信息 用户名， 密码， 数据库名称
static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};

//redis 服务器ip、端口
static char redis_ip[30] = {0};
static char redis_port[10] = {0};

//读取配置信息
void read_cfg();
//解析上传的post数据，上传文件相关
int recv_save_file(long len, char *user, char *filename, char *md5, long *p_size);
//将一个本地文件上传到 fastDFS 后台分布式文件系统中
int upload_to_dstorage(char *filename, char *fileid);
//封装文件存储在分布式系统中的完整 url
int make_file_url(char *fileid, char *fdfs_file_url);
//将该文件的FastDFS相关信息存入mysql中
int store_fileinfo_to_mysql(char *user, char *filename, char *md5, long size, char *fileid, char *fdfs_file_url);

int main()
{
    char filename[FILE_NAME_LEN] = {0}; //文件名
    char user[USER_NAME_LEN] = {0};   	//文件上传者
    char md5[MD5_LEN] = {0};    		//文件md5码
    long size;  						//文件大小
    char fileid[TEMP_BUF_MAX_LEN] = {0};    //文件上传到fastDFS后的文件id
    char fdfs_file_url[FILE_URL_LEN] = {0}; //文件所存放storage的host_name

    //读取数据库配置信息
    read_cfg();

    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main():执行上传文件事件的CGI程序\n");

    while (FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        long len;
        int ret = 0;

        printf("Content-type: text/plain\r\n");

        if( contentLength == NULL )
        {
            len = 0;
        }
        else
        {
        	//字符串转整型
            len = atoi(contentLength); 
        }

#if 0
        if (len <= 0)
        {
            char* out = return_status("017");
        }
        else
        {
            char* out = return_status("004");
            if(out != NULL)
            {
                printf(out);
                free(out);
            }
        }

#else
        if (len <= 0)
        {
            printf("No data from standard input\n");
            LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main():没有登陆用户信息\n");
        }
        else
        {
            //获取上传文件
            if (recv_save_file(len, user, filename, md5, &size) < 0)
            {
                LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main():获取上传文件时发生错误\n");
                ret = -1;
                goto END;
            }

            //将该文件存入fastDFS中,并得到文件的file_id
            if (upload_to_dstorage(filename, fileid) < 0)
            {
                LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main():将该文件存入fastDFS时发生错误\n");
                ret = -1;
                goto END;
            }

            //删除本地临时存放的上传文件
            unlink(filename);

            //得到文件所存放storage的host_name
            if (make_file_url(fileid, fdfs_file_url) < 0)
            {
                LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main():得到文件url时发生错误\n");
                ret = -1;
                goto END;
            }

            //将该文件的FastDFS相关信息存入mysql中
            if (store_fileinfo_to_mysql(user, filename, md5, size, fileid, fdfs_file_url) < 0)
            {
                LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main():将该文件的相关信息存入mysql中时发生错误\n");
                ret = -1;
                goto END;
            }


END:
            memset(filename, 0, FILE_NAME_LEN);
            memset(user, 0, USER_NAME_LEN);
            memset(md5, 0, MD5_LEN);
            memset(fileid, 0, TEMP_BUF_MAX_LEN);
            memset(fdfs_file_url, 0, FILE_URL_LEN);

            char *out = NULL;
            //给前端返回，上传情况
            /*
               上传文件：
               成功：{"code":"004"}
               失败：{"code":"017"}
               */
            if(ret == 0) //成功上传
            {
                out = return_status("004");
            }
            else//上传失败
            {
                out = return_status("017");
            }

            if(out != NULL)
            {
                printf(out);
                free(out);
            }

        }
#endif

    } /* while */

    return 0;
}

//读取配置信息
void read_cfg()
{
    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main() --> read_cfg()\n");

    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);
}

//解析上传的post数据
int recv_save_file(long len, char *user, char *filename, char *md5, long *p_size)
{
    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main() --> recv_save_file()\n");

    int ret = 0;
    char *file_buf = NULL;
    char *begin = NULL;
    char *p, *q, *k;

    //文件头部信息
    char content_text[TEMP_BUF_MAX_LEN] = {0}; //TEMP_BUF_MAX_LEN：512 临时缓冲区大小
    //分界线信息
    char boundary[TEMP_BUF_MAX_LEN] = {0};     

    //开辟存放文件的内存
    file_buf = (char *)malloc(len);
    if (file_buf == NULL)
    {
        return -1;
    }

    //从服务端读取内容
    int ret2 = fread(file_buf, 1, len, stdin); 
    if(ret2 == 0)
    {
        ret = -1;
        goto END;
    }

    //处理客户端发送过来的post数据格式
    begin = file_buf;    //内存起点
    p = begin;

    /*
       ------WebKitFormBoundary88asdgewtgewx\r\n
       Content-Disposition: form-data; user="xxx"; filename="xxx"; md5="xxxx"; size=xxx\r\n
       Content-Type: application/octet-stream\r\n
       \r\n
       真正的文件内容\r\n
       ------WebKitFormBoundary88asdgewtgewx
    */

    //得到分界线结束位置
    p = strstr(begin, "\r\n");
    if (p == NULL)
    {
        ret = -1;
        goto END;
    }
    //拷贝分界线
    strncpy(boundary, begin, p-begin);
    //字符串结束符
    boundary[p-begin] = '\0';
    p += 2;	//跳过\r\n，指向 Content-Disposition 一行
    //已经处理了p-begin的长度
    len -= (p-begin);

    //重新定位起点
    begin = p;
    //开始处理：Content-Disposition
    p = strstr(begin, "\r\n");
    if(p == NULL)
    {
        ret = -1;
        goto END;
    }
    //将文件信息保存在 content_text
    strncpy(content_text, begin, p-begin);
    content_text[p-begin] = '\0';
    p += 2;	//\r\n
    len -= (p-begin);

    //获取文件上传者 user="xxx"
    q = begin;
    q = strstr(begin, "user=");	//查找 user= 的位置
    q += strlen("user=");		//跳过 user=
    q++;    					//跳过 第一个" 
    k = strchr(q, '"');			//查找 第二个" 的位置
    strncpy(user, q, k-q);		//拷贝用户名，k-q即为用户名的长度
    user[k-q] = '\0';
    //去掉一个字符串两边的空白字符
    trim_space(user);

    //获取文件名字 filename="xxx"
    begin = k;
    q = begin;
    q = strstr(begin, "filename=");	//查找 filename= 的位置
    q += strlen("filename=");		//跳过 filename=
    q++;    						//跳过 第一个" 
    k = strchr(q, '"');				//查找 第二个" 的位置
    strncpy(filename, q, k-q);		//拷贝文件名
    filename[k-q] = '\0';
    trim_space(filename);

    //获取文件MD5码  md5="xxx"
    begin = k;
    q = begin;
    q = strstr(begin, "md5=");		//查找 md5= 的位置
    q += strlen("md5=");			//跳过 md5=
    q++;    						//跳过 第一个" 
    k = strchr(q, '"');				//查找 第二个" 的位置
    strncpy(md5, q, k-q);			//拷贝md5
    trim_space(md5);

    //获取文件大小 size=xxx
    begin = k;
    q = begin;
    q = strstr(begin, "size=");
	q += strlen("size=");
	k = strstr(q, "\r\n");
    char tmp[256] = {0};
    strncpy(tmp, q, k-q);
    tmp[k-q] = '\0';
    *p_size = strtol(tmp, NULL, 10); //字符串转long

    //重新定位 begin 的位置
    k += 2;	//跳过\r\n，指向 Content-Type 一行的开始位置
    begin = k;
    //开始处理：Content-Type
    p = strstr(begin, "\r\n");	//指向 Content-Type 一行的 \r\n
    p += 4;	//\r\n\r\n，指向真正的文件内容
    len -= (p-begin);

    //处理文件的真正内容
    begin = p;
    //通过分界线找文件结尾
    p = memstr(begin, len, boundary);
    if (p == NULL)
    {
        ret = -1;
        goto END;
    }
    else
    {
        p = p - 2;//\r\n
    }

    //此时 begin和p 两个指针的区间就是post的文件二进制数据
    //将数据写入文件中
    int fd = 0;
    fd = open(filename, O_CREAT|O_WRONLY, 0644);
    if (fd < 0)
    {
        ret = -1;
        goto END;
    }

    //ftruncate会将参数fd指定的文件大小改为参数length指定的大小
    ftruncate(fd, (p-begin));
    write(fd, begin, (p-begin));
    close(fd);

END:
    free(file_buf);
    return ret;
}

//将一个本地文件上传到 fastDFS 后台分布式文件系统中
int upload_to_dstorage(char *filename, char *fileid)
{
    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main() --> upload_to_dstorage()\n");

    int ret = 0;
    pid_t pid;
    int fd[2];

    //创建管道
    if (pipe(fd) < 0)
    {
        ret = -1;
        goto END;
    }

    //创建进程
    pid = fork();
    if (pid < 0)
    {
        ret = -1;
        goto END;
    }

    if(pid == 0) //子进程
    {
        //关闭读端
        close(fd[0]);

        //将标准输出 重定向 写管道
        dup2(fd[1], STDOUT_FILENO);	//dup2(fd[1], 1);

        //读取fdfs client 配置文件的路径
        char fdfs_cli_conf_path[256] = {0};
        get_cfg_value(CFG_PATH, "dfs_path", "client", fdfs_cli_conf_path);

        //通过execlp执行fdfs_upload_file
        execlp("fdfs_upload_file", "fdfs_upload_file", fdfs_cli_conf_path, filename, NULL);

        close(fd[1]);
    }
    else //父进程
    {
        //关闭写端
        close(fd[1]);

        //从管道中去读数据
        read(fd[0], fileid, TEMP_BUF_MAX_LEN);

        //去掉一个字符串两边的空白字符
        trim_space(fileid);

        if (strlen(fileid) == 0)
        {
            ret = -1;
            goto END;
        }

        //等待子进程结束，回收其资源
        wait(NULL); 	
        close(fd[0]);
    }

END:
    return ret;
}

//封装文件存储在分布式系统中的完整 url
int make_file_url(char *fileid, char *fdfs_file_url)
{
    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main() --> make_file_url()\n");

    int ret = 0;

    char *p = NULL;
    char *q = NULL;
    char *k = NULL;

    char fdfs_file_stat_buf[TEMP_BUF_MAX_LEN] = {0};
    char fdfs_file_host_name[HOST_NAME_LEN] = {0};  //storage所在服务器ip地址

/*
    pid_t pid;
    int fd[2];

    //创建管道
    if (pipe(fd) < 0)
    {
        ret = -1;
        goto END;
    }

    //创建进程
    pid = fork();
    if (pid < 0)
    {
        ret = -1;
        goto END;
    }

    if(pid == 0) //子进程
    {
        //关闭读端
        close(fd[0]);

        //将标准输出 重定向 写管道
        dup2(fd[1], STDOUT_FILENO); //dup2(fd[1], 1);

        //读取fdfs client 配置文件的路径
        char fdfs_cli_conf_path[256] = {0};
        get_cfg_value(CFG_PATH, "dfs_path", "client", fdfs_cli_conf_path);

        execlp("fdfs_file_info", "fdfs_file_info", fdfs_cli_conf_path, fileid, NULL);

        close(fd[1]);
    }
    else //父进程
    {
        //关闭写端
        close(fd[1]);

        //从管道中去读数据
        read(fd[0], fdfs_file_stat_buf, TEMP_BUF_MAX_LEN);
       	//等待子进程结束，回收其资源
        wait(NULL); 
        close(fd[0]);

        //拼接上传文件的完整url地址--->http://host_name/group1/M00/00/00/D12313123232312.png
        p = strstr(fdfs_file_stat_buf, "source ip address: ");

        q = p + strlen("source ip address: ");
        k = strstr(q, "\n");

        strncpy(fdfs_file_host_name, q, k-q);
        fdfs_file_host_name[k-q] = '\0';


        //读取storage_web_server服务器的端口
        char storage_web_server_port[20] = {0};
        get_cfg_value(CFG_PATH, "storage_web_server", "port", storage_web_server_port);
        strcat(fdfs_file_url, "http://");
        strcat(fdfs_file_url, fdfs_file_host_name);
        strcat(fdfs_file_url, ":");
        strcat(fdfs_file_url, storage_web_server_port);
        strcat(fdfs_file_url, "/");
        strcat(fdfs_file_url, fileid);

    }
*/
	char storage_web_server_port[20] = {0};
	get_cfg_value(CFG_PATH, "storage_web_server", "port", storage_web_server_port);
    get_cfg_value(CFG_PATH, "storage_web_server", "ip", fdfs_file_host_name);
    strcat(fdfs_file_url, "http://");
    strcat(fdfs_file_url, fdfs_file_host_name);
    strcat(fdfs_file_url, ":");
    strcat(fdfs_file_url, storage_web_server_port);
    strcat(fdfs_file_url, "/");
    strcat(fdfs_file_url, fileid);

END:
    return ret;
}

//将该文件的FastDFS相关信息存入mysql中
int store_fileinfo_to_mysql(char *user, char *filename, char *md5, long size, char *fileid, char *fdfs_file_url)
{
    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "main() --> store_fileinfo_to_mysql()\n");
    
    int ret = 0;
    MYSQL *conn = NULL; //数据库连接句柄

    time_t now;;
    char create_time[TIME_STRING_LEN];
    char suffix[SUFFIX_LEN];    //文件后缀
    char sql_cmd[SQL_MAX_LEN] = {0};

    //连接 mysql 数据库
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
		LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "store_fileinfo_to_mysql() --> msql_conn() error\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码
    mysql_query(conn, "set names utf8");

    //得到文件后缀字符串 如果非法文件后缀,返回"null"
    get_file_suffix(filename, suffix); //mp4, jpg, png

    //sql 语句
    /*
       文件信息表
       -- md5 文件md5
       -- file_id 文件id
       -- url 文件url
       -- size 文件大小, 以字节为单位
       -- type 文件类型： png, zip, mp4……
       -- count 文件引用计数， 默认为1， 每增加一个用户拥有此文件，此计数器+1
       */
    sprintf(sql_cmd, "insert into file_info (md5, file_id, url, size, type, count) values ('%s', '%s', '%s', '%ld', '%s', %d)",
            md5, fileid, fdfs_file_url, size, suffix, 1);

    if (mysql_query(conn, sql_cmd) != 0) //执行sql语句
    {
		LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "store_fileinfo_to_mysql() --> mysql_query() error\n");
        ret = -1;
        goto END;
    }

    //获取当前时间
    now = time(NULL);
    strftime(create_time, TIME_STRING_LEN-1, "%Y-%m-%d %H:%M:%S", localtime(&now));

    /*
       用户文件列表
       -- user 文件所属用户
       -- md5 文件md5
       -- createtime 文件创建时间
       -- filename 文件名字
       -- pv 文件下载量，默认值为0，下载一次加1
       */
    //sql语句
    sprintf(sql_cmd, "insert into user_file_list (user, md5, createtime, filename, pv) values ('%s', '%s', '%s', '%s', %d)", user, md5, create_time, filename, 0);
    if(mysql_query(conn, sql_cmd) != 0)
    {
		LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "store_fileinfo_to_mysql() --> mysql_query() error\n");
        ret = -1;
        goto END;
    }

    //查询用户文件数量
    sprintf(sql_cmd, "select count from user_file_count where user = '%s'", user);
    int ret2 = 0;
    char tmp[512] = {0};
    int count = 0;
    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    ret2 = process_result_one(conn, sql_cmd, tmp); //执行sql语句
    if(ret2 == 1) //没有记录
    {
        //插入记录
        sprintf(sql_cmd, " insert into user_file_count (user, count) values('%s', %d)", user, 1);
    }
    else if(ret2 == 0)
    {
        //更新用户文件数量count字段
        count = atoi(tmp);
        sprintf(sql_cmd, "update user_file_count set count = %d where user = '%s'", count+1, user);
    }


    if(mysql_query(conn, sql_cmd) != 0)
    {
		LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "store_fileinfo_to_mysql() --> mysql_query() error\n");
        ret = -1;
        goto END;
    }

END:
    if (conn != NULL)
    {
        mysql_close(conn); //断开数据库连接
    }

    return ret;
}
