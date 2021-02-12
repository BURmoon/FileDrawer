/*
 * 删除文件、文件pv字段处理 相关的后台CGI程序
 */

#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util_cgi.h"
#include "deal_mysql.h"
#include "redis_keys.h"
#include "redis_op.h"
#include "cfg.h"
#include "cJSON.h"
#include "make_log.h"
#include <sys/time.h>

#define DEALFILE_LOG_MODULE       "cgi"
#define DEALFILE_LOG_PROC         "dealfile"

//mysql 数据库配置信息 用户名， 密码， 数据库名称
static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};

//redis 服务器ip、端口
static char redis_ip[30] = {0};
static char redis_port[10] = {0};

//读取配置信息
void read_cfg();
//解析的json包
int get_json_info(char *buf, char *user, char *token, char *md5, char *filename);
//删除文件
int del_file(char *user, char *md5, char *filename);
//从storage删除指定的文件，参数为文件id
int remove_file_from_storage(char *fileid);
//文件下载标志处理
int pv_file(char *user, char *md5, char *filename);

int main()
{
    char cmd[20];
    char user[USER_NAME_LEN];		//用户名
    char token[TOKEN_LEN];			//token
    char md5[MD5_LEN];				//文件md5码
    char filename[FILE_NAME_LEN];	//文件名字

    //读取数据库配置信息
    read_cfg();

    LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main():执行文件处理事件的CGI程序\n");

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {
        //获取URL地址 "?" 后面的内容
        char *query = getenv("QUERY_STRING");

        //解析命令
        query_parse_key_value(query, "cmd", cmd, NULL);

        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        printf("Content-type: text/html\r\n\r\n");

        if( contentLength == NULL )
        {
            len = 0;
        }
        else
        {
            len = atoi(contentLength);
        }

#if 0
        if (len <= 0)
        {
            printf("No data from standard input\n");
        }
        else
        {
            if(strcmp(cmd, "del") == 0) //删除文件
            {
                char* out = return_status("006");
                if(out != NULL)
                {
                printf(out);
                free(out);
                }
            }
            else if(strcmp(cmd, "pv") == 0) //文件下载标志处理
            {
                char* out = return_status("005");
                if(out != NULL)
                {
                printf(out);
                free(out);
                }
            }
        }

#else
        if (len <= 0)
        {
            printf("No data from standard input\n");
            LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main():没有用户登录信息\n");
        }
        else
        {
            char buf[4*1024] = {0};
            int ret = 0;
            ret = fread(buf, 1, len, stdin); //从标准输入(web服务器)读取内容
            if(ret == 0)
            {
                LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main():读取用户登录信息失败，fread()返回值为 0\n");
                continue;
            }

            get_json_info(buf, user, token, md5, filename); //解析json信息
            LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main():解析客户端json信息 %s-%s\n", user, filename);
            //验证登陆token，成功返回0，失败-1
            ret = verify_token(user, token); //util_cgi.h
            if(ret != 0)
            {
                char *out = return_status("111"); //token验证失败错误码
                if(out != NULL)
                {
                    printf(out); //给前端反馈错误码
                    free(out);
                }
                continue;   //跳过本次循环
            }
            LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main():验证登陆token，验证结果为 %d\n", ret);

            if(strcmp(cmd, "del") == 0) //删除文件
            {
                LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main():执行删除文件任务\n");
                del_file(user, md5, filename);
            }
            else if(strcmp(cmd, "pv") == 0) //文件下载标志处理
            {
                LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main():执行文件下载标志处理任务\n");
                pv_file(user, md5, filename);
            }
        }
#endif
    }

    return 0;
}

//读取配置信息
void read_cfg()
{
    LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main() --> read_cfg()\n");

    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);
}

//解析的json包
int get_json_info(char *buf, char *user, char *token, char *md5, char *filename)
{
    LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main() --> get_json_info()\n");

    int ret = 0;

    //json数据 { user:xxx , token:xxx , md5:xxx , fileName:xxx }

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "get_json_info() --> cJSON_Parse() error\n");
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    //用户
    cJSON *child1 = cJSON_GetObjectItem(root, "user");
    if(NULL == child1)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "get_json_info() --> cJSON_GetObjectItem(user) error\n");
        ret = -1;
        goto END;
    }
    strcpy(user, child1->valuestring); //拷贝内容

    //文件md5码
    cJSON *child2 = cJSON_GetObjectItem(root, "md5");
    if(NULL == child2)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "get_json_info() --> cJSON_GetObjectItem(md5) error\n");
        ret = -1;
        goto END;
    }
    strcpy(md5, child2->valuestring); //拷贝内容

    //文件名字
    cJSON *child3 = cJSON_GetObjectItem(root, "fileName");
    if(NULL == child3)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "get_json_info() --> cJSON_GetObjectItem(fileName) error\n");
        ret = -1;
        goto END;
    }
    strcpy(filename, child3->valuestring); //拷贝内容

    //token
    cJSON *child4 = cJSON_GetObjectItem(root, "token");
    if(NULL == child4)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "get_json_info() --> cJSON_GetObjectItem(token) error\n");
        ret = -1;
        goto END;
    }
    strcpy(token, child4->valuestring); //拷贝内容


END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

//删除文件
int del_file(char *user, char *md5, char *filename)
{
    LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main() --> del_file()\n");

    //从mysql中查询(mysql操作)如果有记录，需要删除相关记录
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    char *out = NULL;
    char tmp[512] = {0};
    char fileid[1024] = {0};
    int ret2 = 0;
    int count = 0;

    //连接数据库
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> msql_conn() error\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    //用户文件数量-1
    //查询用户文件数量
    sprintf(sql_cmd, "select count from user_file_count where user = '%s'", user);
    ret2 = process_result_one(conn, sql_cmd, tmp); //执行sql语句
    if(ret2 != 0)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> process_result_one() error\n");
        ret = -1;
        goto END;
    }
    count = atoi(tmp);
    sprintf(sql_cmd, "update user_file_count set count = %d where user = '%s'", count-1, user);

/*
    //更改用户文件数量表 user_file_count
    if(count == 1)
    {
        //删除用户文件数量表对应的数据
        sprintf(sql_cmd, "delete from user_file_count where user = '%s'", user);
    }
    else
    {
        sprintf(sql_cmd, "update user_file_count set count = %d where user = '%s'", count-1, user);
    }
*/

    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> mysql_query() error\n");
        ret = -1;
        goto END;
    }

    //更改用户文件列表 user_file_list，删除该文件
    sprintf(sql_cmd, "delete from user_file_list where user = '%s' and md5 = '%s' and filename = '%s'", user, md5, filename);
    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> mysql_query() error\n");
        ret = -1;
        goto END;
    }

    //更改文件信息表 file_info，文件引用计数count -1
    //查看该文件文件引用计数
    sprintf(sql_cmd, "select count from file_info where md5 = '%s'", md5);
    ret2 = process_result_one(conn, sql_cmd, tmp); //执行sql语句
    if(ret2 == 0)
    {
        count = atoi(tmp); //count字段
    }
    else
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> process_result_one() error\n");
        ret = -1;
        goto END;
    }

    count--; //减一
    sprintf(sql_cmd, "update file_info set count=%d where md5 = '%s'", count, md5);
    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> mysql_query() error\n");
        ret = -1;
        goto END;
    }

    //当没有用户引用此文件，需要在storage删除此文件
    if(count == 0) 
    {
        //查询文件的id
        sprintf(sql_cmd, "select file_id from file_info where md5 = '%s'", md5);
        ret2 = process_result_one(conn, sql_cmd, tmp); //执行sql语句
        if(ret2 != 0)
        {
            LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> process_result_one() error\n");
            ret = -1;
            goto END;
        }

        //删除文件信息表中该文件的信息
        sprintf(sql_cmd, "delete from file_info where md5 = '%s'", md5);
        if (mysql_query(conn, sql_cmd) != 0)
        {
            LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> mysql_query() error\n");
            ret = -1;
            goto END;
        }

        //从storage服务器删除此文件，参数为为文件id
        ret2 = remove_file_from_storage(tmp);
        if(ret2 != 0)
        {
            LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "del_file() --> remove_file_from_storage() error\n");
            ret = -1;
            goto END;
        }
    }


END:
    /*
    删除文件：
        成功：{"code":"006"}
        失败：{"code":"019"}
    */
    out = NULL;
    if(ret == 0)
    {
        out = return_status("006");
    }
    else
    {
        out = return_status("019");
    }

    if(out != NULL)
    {
        printf(out);
        free(out);
    }

    if(conn != NULL)
    {
        mysql_close(conn);
    }

    return ret;
}

//从storage删除指定的文件，参数为文件id
int remove_file_from_storage(char *fileid)
{
    LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main() --> remove_file_from_storage()\n");
    int ret = 0;

    //读取fdfs client 配置文件的路径
    char fdfs_cli_conf_path[256] = {0};
    get_cfg_value(CFG_PATH, "dfs_path", "client", fdfs_cli_conf_path);

    char cmd[1024*2] = {0};
    sprintf(cmd, "fdfs_delete_file %s %s", fdfs_cli_conf_path, fileid);

    ret = system(cmd);

    return ret;
}

//文件下载标志处理
int pv_file(char *user, char *md5, char *filename)
{
    LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "main() --> pv_file()\n");
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    char *out = NULL;
    char tmp[512] = {0};
    int ret2 = 0;

    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "mpv_fileain() --> msql_conn()\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    //sql语句，查看该文件的pv字段
    sprintf(sql_cmd, "select pv from user_file_list where user = '%s' and md5 = '%s' and filename = '%s'", user, md5, filename);
    ret2 = process_result_one(conn, sql_cmd, tmp); //执行sql语句
    int pv = 0;
    if(ret2 == 0)
    {
        pv = atoi(tmp); //pv字段
    }
    else
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "mpv_fileain() --> process_result_one()\n");
        ret = -1;
        goto END;
    }

    //更新该文件pv字段，+1
    sprintf(sql_cmd, "update user_file_list set pv = %d where user = '%s' and md5 = '%s' and filename = '%s'", pv+1, user, md5, filename);

    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "mpv_fileain() --> mysql_query()\n");
        ret = -1;
        goto END;
    }

END:
    /*
    下载文件pv字段处理
        成功：{"code":"005"}
        失败：{"code":"018"}
    */
    out = NULL;
    if(ret == 0)
    {
        out = return_status("005");
    }
    else
    {
        out = return_status("018");
    }

    if(out != NULL)
    {
        printf(out);
        free(out);
    }


    if(conn != NULL)
    {
        mysql_close(conn);
    }

    return ret;
}
