/*
 * 用户列表相关的后台CGI程序
 */

#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util_cgi.h"
#include "deal_mysql.h"
#include "cfg.h"
#include "cJSON.h"
#include "redis_op.h"
#include "make_log.h"
#include <sys/time.h>

#define MYFILES_LOG_MODULE       "cgi"
#define MYFILES_LOG_PROC         "myfiles"

//mysql 数据库配置信息 用户名， 密码， 数据库名称
static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};

//redis 服务器ip、端口
static char redis_ip[30] = {0};
static char redis_port[10] = {0};

//读取配置信息
void read_cfg();
//解析的json包, 登陆token
int get_count_json_info(char *buf, char *user, char *token);
//验证登陆token，成功返回0，失败-1
//int verify_token(char *user, char *token);
//获取用户文件个数
void get_user_files_count(char *user);
//返回给客户端的情况
void return_login_status(long num, int token_flag);
//解析的json包
int get_fileslist_json_info(char *buf, char *user, char *token, int *p_start, int *p_count);
//获取用户文件列表
int get_user_filelist(char *cmd, char *user, int start, int count);
//返回情况，NULL代表失败, 返回的指针不为空，则需要free
//char* return_status(char *status_num);

int main()
{
    //count 获取用户文件个数
    //normal 获取用户文件信息，展示到前端
    char cmd[20];
    char user[USER_NAME_LEN];	//128 用户名字长度
    char token[TOKEN_LEN];		//128 登陆token长度

     //读取数据库配置信息
    read_cfg();

    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main():执行用户文件处理事件的CGI程序\n");

    //阻塞等待用户连接
    while (FCGI_Accept() >= 0)
    {

        //获取URL地址 "?" 后面的内容
        char *query = getenv("QUERY_STRING");

        //解析命令
        query_parse_key_value(query, "cmd", cmd, NULL);
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main():解析命令cmd= %s\n", cmd);

        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        printf("Content-type: text/html\r\n\r\n");

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
            printf("No data from standard input\n");    
        }
        else
        {
            char buf[4*1024] = {0};
            int ret = 0;
            ret = fread(buf, 1, len, stdin);
            if(ret == 0)
            {
                continue;
            }

            if (strcmp(cmd, "count") == 0) //count 获取用户文件个数
            {
                return_login_status(1, 0);
            }
            //获取用户文件信息 127.0.0.1:80/myfiles&cmd=normal
            else
            {
                cJSON* root = cJSON_CreateObject();
                cJSON* array = cJSON_CreateArray();
                cJSON* item = cJSON_CreateObject();

                cJSON_AddStringToObject(item, "user", "TEST");
                cJSON_AddStringToObject(item, "md5", "e8ea6031b779ac26c319ddf949ad9d8d");
                cJSON_AddStringToObject(item, "time", "1111-11-11 11:11:11");
                cJSON_AddStringToObject(item, "fileName", "test.txt");
                cJSON_AddNumberToObject(item, "pv", 0);
                cJSON_AddStringToObject(item, "url", "http://127.0.0.1:80/group1/M00/00/00/test.txt");
                cJSON_AddNumberToObject(item, "size", 27473666);
                cJSON_AddStringToObject(item, "type", "txt");

                cJSON_AddItemToArray(array, item);

                cJSON* item1 = cJSON_CreateObject();
                cJSON_AddStringToObject(item1, "user", "TEST");
                cJSON_AddStringToObject(item1, "md5", "e8ea6031b779ac26c319ddf949ad9d8d");
                cJSON_AddStringToObject(item1, "time", "2222-22-22 11:11:11");
                cJSON_AddStringToObject(item1, "fileName", "test.jpg");
                cJSON_AddNumberToObject(item1, "pv", 0);
                cJSON_AddStringToObject(item1, "url", "http://127.0.0.1:80/group1/M00/00/00/test.txt");
                cJSON_AddNumberToObject(item1, "size", 27473666);
                cJSON_AddStringToObject(item1, "type", "jpg");
                cJSON_AddItemToArray(array, item1);

                cJSON* item2 = cJSON_CreateObject();
                cJSON_AddStringToObject(item2, "user", "TEST");
                cJSON_AddStringToObject(item2, "md5", "e8ea6031b779ac26c319ddf949ad9d8d");
                cJSON_AddStringToObject(item2, "time", "3333-33-33 11:11:11");
                cJSON_AddStringToObject(item2, "fileName", "test.ppt");
                cJSON_AddNumberToObject(item2, "pv", 0);
                cJSON_AddStringToObject(item2, "url", "http://127.0.0.1:80/group1/M00/00/00/test.txt");
                cJSON_AddNumberToObject(item2, "size", 27473666);
                cJSON_AddStringToObject(item2, "type", "ppt");
                cJSON_AddItemToArray(array, item2);
                
                cJSON_AddItemToObject(root, "files", array);

                char* out = cJSON_Print(root);
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
            LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main():没有登陆用户信息\n");
        }
        else
        {
            char buf[4*1024] = {0};
            int ret = 0;
            ret = fread(buf, 1, len, stdin);
            if(ret == 0)
            {
                LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main():读取用户登录信息失败，fread()返回值为 0\n");
                continue;
            }

            if (strcmp(cmd, "count") == 0) //count 获取用户文件个数
            {
                get_count_json_info(buf, user, token); //通过json包获取用户名, token
                LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main():读取登陆用户信息 %s\n", user);

                //验证登陆token，成功返回0，失败-1
                ret = verify_token(user, token);
                if(ret == 0)
                {
                    //获取用户文件个数，并返回给客户端
                    get_user_files_count(user);
                    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main():获取用户文件个数\n");
                }
                else
                {
                    char *out = return_status("111"); //token验证失败错误码
                    if(out != NULL)
                    {
                        printf(out);
                        free(out);
                    }
                }
            }
            //获取用户文件信息 127.0.0.1:80/myfiles&cmd=normal
            else
            {
                int start; //文件起点
                int count; //文件个数
                get_fileslist_json_info(buf, user, token, &start, &count); //通过json包获取信息
                LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main():读取登陆用户信息 %s\n", user);

                //验证登陆token，成功返回0，失败-1
                ret = verify_token(user, token); //util_cgi.h
                if(ret == 0)
                {
                     get_user_filelist(cmd, user, start, count); //获取用户文件列表
                     LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main():获取用户文件列表\n");
                }
                else
                {
                    char *out = return_status("111"); //token验证失败错误码
                    if(out != NULL)
                    {
                        printf(out);
                        free(out);
                    }
                }

            }

        }

#endif

    }

    return 0;
}

//读取配置信息
void read_cfg()
{
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main() --> read_cfg()\n");

    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);
}

//解析的json包, 登陆token
int get_count_json_info(char *buf, char *user, char *token)
{
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main() --> get_count_json_info()\n");

    int ret = 0;

    //json数据 { user:xxx , token:xxx }

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    //用户
    cJSON *child1 = cJSON_GetObjectItem(root, "user");
    if(NULL == child1)
    {
        ret = -1;
        goto END;
    }
    strcpy(user, child1->valuestring); //拷贝内容

    //登陆token
    cJSON *child2 = cJSON_GetObjectItem(root, "token");
    if(NULL == child2)
    {
        ret = -1;
        goto END;
    }
    strcpy(token, child2->valuestring); //拷贝内容

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

/*
//验证登陆token，成功返回0，失败-1
int verify_token(char *user, char *token)
{
    int ret = 0;
    redisContext * redis_conn = NULL;
    char tmp_token[128] = {0};

    //连接redis数据库
    redis_conn = rop_connectdb_nopwd(redis_ip, redis_port);
    if (redis_conn == NULL)
    {
        ret = -1;
        goto END;
    }

    //获取user对应的value
    ret = rop_get_string(redis_conn, user, tmp_token);
    if(ret == 0)
    {
        if( strcmp(token, tmp_token) != 0 ) //token不相等
        {
            ret = -1;
        }
    }

END:
    if(redis_conn != NULL)
    {
        rop_disconnect(redis_conn);
    }

    return ret;
}
*/

//获取用户文件个数
void get_user_files_count(char *user)
{
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main() --> get_user_files_count()\n");

    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    long line = 0;

    //connect the database
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "get_user_files_count() --> mysql_conn() error\n");
        goto END;
    }

    mysql_query(conn, "set names utf8");

    sprintf(sql_cmd, "select count from user_file_count where user=\"%s\"", user);
    char tmp[512] = {0};
    //返回值： 0成功并保存记录集，1没有记录集，2有记录集但是没有保存，-1失败
    int ret2 = process_result_one(conn, sql_cmd, tmp); //指向sql语句
    if(ret2 != 0)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "get_user_files_count() --> process_result_one() return %d\n", ret2);
        goto END;
    }

    line = atol(tmp); //字符串转长整形

END:
    if(conn != NULL)
    {
        mysql_close(conn);
    }

    //给客户端反馈的信息
    return_login_status(line, ret2);
}

//返回客户端情况
void return_login_status(long num, int token_flag)
{
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main() --> return_login_status()\n");

    char *out = NULL;
    char *token;
    char num_buf[128] = {0};

    if(token_flag == 0)
    {
        token = "002"; //成功
    }
    else
    {
        token = "013"; //失败
    }

    //数字
    sprintf(num_buf, "%ld", num);

    cJSON *root = cJSON_CreateObject();  //创建json项目
    cJSON_AddStringToObject(root, "num", num_buf);
    cJSON_AddStringToObject(root, "code", token);
    out = cJSON_Print(root);    //cJSON to string(char *)

    cJSON_Delete(root);

    if(out != NULL)
    {
        printf(out); //给客户端反馈的信息
        free(out);
    }
}

//解析的json包
int get_fileslist_json_info(char *buf, char *user, char *token, int *p_start, int *p_count)
{
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main() --> get_fileslist_json_info()\n");

    int ret = 0;

    //json数据 { user:xxx , token:xxx , start:xxx , count:xxx }

    //解析json包
    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象
    //用户
    cJSON *child1 = cJSON_GetObjectItem(root, "user");
    if(NULL == child1)
    {
        ret = -1;
        goto END;
    }
    strcpy(user, child1->valuestring); //拷贝内容

    //token
    cJSON *child2 = cJSON_GetObjectItem(root, "token");
    if(NULL == child2)
    {
        ret = -1;
        goto END;
    }
    strcpy(token, child2->valuestring); //拷贝内容

    //文件起点
    cJSON *child3 = cJSON_GetObjectItem(root, "start");
    if(NULL == child3)
    {
        ret = -1;
        goto END;
    }
    *p_start = child3->valueint;

    //文件请求个数
    cJSON *child4 = cJSON_GetObjectItem(root, "count");
    if(NULL == child4)
    {
        ret = -1;
        goto END;
    }
    *p_count = child4->valueint;

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

//获取用户文件列表
int get_user_filelist(char *cmd, char *user, int start, int count)
{
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "main() --> get_user_filelist()\n");
    
    int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    MYSQL *conn = NULL;
    cJSON *root = NULL;
    cJSON *array =NULL;
    char *out = NULL;
    char *out2 = NULL;
    MYSQL_RES *res_set = NULL;

    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        ret = -1;
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

	//获取用户文件信息
	sprintf(sql_cmd, "select user_file_list.*, file_info.url, file_info.size, file_info.type from file_info, user_file_list where user = '%s' and file_info.md5 = user_file_list.md5 limit %d, %d", user, start, count);
    if (mysql_query(conn, sql_cmd) != 0)
    {
		LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "get_user_filelist() --> mysql_query() error\n");
        ret = -1;
        goto END;
    }

	//生成结果集
    res_set = mysql_store_result(conn);
    if (res_set == NULL)
    {
		LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "get_user_filelist() --> mysql_store_result() error\n");
        ret = -1;
        goto END;
    }

    ulong line = 0;
    //mysql_num_rows接受由mysql_store_result返回的结果结构集，并返回结构集中的行数
    line = mysql_num_rows(res_set);
    if (line == 0)	//没有结果
    {
		LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "get_user_filelist() --> mysql_num_rows() error\n");
        ret = -1;
        goto END;
    }
	LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "get_user_filelist() --> mysql_num_rows() 获得 %d 条结果\n", line);

    MYSQL_ROW row;
    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    //mysql_fetch_row从使用mysql_store_result得到的结果结构中提取一行，并把它放到一个行结构中。
    //当数据用完或发生错误时返回NULL.
    while ((row = mysql_fetch_row(res_set)) != NULL)
    {
        //array[i]:
        cJSON* item = cJSON_CreateObject();
        
        //user	文件所属用户
        if(row[0] != NULL)
        {
            cJSON_AddStringToObject(item, "user", row[0]);
        }

        //md5 文件md5
        if(row[1] != NULL)
        {
            cJSON_AddStringToObject(item, "md5", row[1]);
        }

        //createtime 文件创建时间
        if(row[2] != NULL)
        {
            cJSON_AddStringToObject(item, "time", row[2]);
        }

        //filename 文件名字
        if(row[3] != NULL)
        {
            cJSON_AddStringToObject(item, "fileName", row[3]);
        }

        //pv 文件下载量，默认值为0，下载一次加1
        if(row[4] != NULL)
        {
            cJSON_AddNumberToObject(item, "pv", atol(row[4]));
        }

        //url 文件url
        if(row[5] != NULL)
        {
            cJSON_AddStringToObject(item, "url", row[5]);
        }

        //size 文件大小, 以字节为单位
        if(row[6] != NULL)
        {
            cJSON_AddNumberToObject(item, "size", atol(row[6]));
        }

        //type 文件类型： png, zip, mp4……
        if(row[7] != NULL)
        {
            cJSON_AddStringToObject(item, "type", row[7]);
        }

        cJSON_AddItemToArray(array, item);
    }

    cJSON_AddItemToObject(root, "files", array);

    out = cJSON_Print(root);

END:
    if(ret == 0)
    {
        printf(out);  //给前端反馈信息
    }
    else
    {   
        //失败，返回{"code": "014"}
        out2 = NULL;
        out2 = return_status("014");
    }

    if(out2 != NULL)
    {
        printf(out2);
        free(out2);
    }

    if(res_set != NULL)
    {
        //完成所有对数据的操作后，调用mysql_free_result来善后处理
        mysql_free_result(res_set);
    }

    if(conn != NULL)
    {
        mysql_close(conn);
    }

    if(root != NULL)
    {
        cJSON_Delete(root);
    }

    if(out != NULL)
    {
        free(out);
    }


    return ret;
}

/*
//返回情况，NULL代表失败, 返回的指针不为空，则需要free
char* return_status(char *status_num)
{
    char *out = NULL;
    cJSON *root = cJSON_CreateObject();  //创建json项目
    cJSON_AddStringToObject(root, "code", status_num);// {"code":"000"}
    out = cJSON_Print(root);//cJSON to string(char *)

    cJSON_Delete(root);

    return out;
}
*/
