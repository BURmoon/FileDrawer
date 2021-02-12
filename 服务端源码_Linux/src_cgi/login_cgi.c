/*
 * 登陆事件相关的后台CGI程序
 */

#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "util_cgi.h"
#include "deal_mysql.h"
#include "redis_op.h"
#include "cfg.h"
#include "cJSON.h"
#include "des.h"    //加密
#include "base64.h" //base64
#include "md5.h"    //md5
#include "make_log.h"

#define LOGIN_LOG_MODULE "cgi"
#define LOGIN_LOG_PROC   "login"

//mysql 数据库配置信息 用户名， 密码， 数据库名称
static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};
//static char mysql_user[128] = {"root"};
//static char mysql_pwd[128] = {"root"};
//static char mysql_db[128] = {"CloudS"};

//redis 服务器ip、端口
static char redis_ip[30] = {0};
static char redis_port[10] = {0};

//读取配置信息
void read_cfg();
//解析用户登陆信息的json包
int get_login_info(char *login_buf, char *user, char *pwd);
//判断用户登陆情况
int check_user_pwd(char *user, char *pwd);
//生成token字符串, 保存到redis数据库
int set_token(char *user, char *token);
//返回客户端情况
void return_login_status(char *status_num, char *token);

int main()
{
    read_cfg();
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "执行登录事件的CGI程序\n");

    while (FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;
        char token[128] = {0};

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
        if (len <= 0)   //没有登陆用户信息
        {
            printf("No data from standard input\n");
        }
        else            //获取登陆用户信息
        {
			char* str = "HELLO";
            return_login_status("000", token);
        }
# else
        if (len <= 0)   //没有登陆用户信息
        {
            printf("No data from standard input\n");
            LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "main():没有登陆用户信息\n");
        }
        else            //获取登陆用户信息
        {
            char buf[4*1024] = {0};
            int ret = 0;
            ret = fread(buf, 1, len, stdin);
            if(ret == 0)
            {
                LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "main():读取用户登录信息失败，fread()返回值为 0\n");
                continue;
            }

            //获取登陆用户的信息
            char user[512] = {0};
            char pwd[512] = {0};
            get_login_info(buf, user, pwd);
            LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "mian():读取登陆用户信息 %s-%s\n", user, pwd);

            //登陆判断，成功返回0，失败返回-1
            ret = check_user_pwd(user, pwd);
            if (ret == 0) //登陆成功
            {
                //生成token字符串
                memset(token, 0, sizeof(token));
                ret = set_token(user, token);
            }
            LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "mian(): 检查登陆用户登录信息，并生成token字符串\n");

            if(ret == 0)
            {
                return_login_status("000", token);
            }
            else	//失败
            {
                return_login_status("010", "fail");
            }
        }

#endif
    }

    return 0;
}

//读取配置信息
void read_cfg()
{
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "read_cfg():从配置文件中读取MySQL和Redis的配置信息\n");

    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);

    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "%s-%s-%s\n", mysql_user, mysql_pwd, mysql_db);
}

//解析用户登陆信息的json包
int get_login_info(char *login_buf, char *user, char *pwd)
{
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "get_login_info():解析包含用户登陆信息的json包\n");

	int ret = 0;

	cJSON * root = cJSON_Parse(login_buf);
    if(NULL == root)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "get_login_info():解析json包时 cJSON_Parse() 发生错误\n");
        ret = -1;
        goto END;
    }

    cJSON *child1 = cJSON_GetObjectItem(root, "user");
    if(NULL == child1)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "get_login_info():解析json包时 cJSON_GetObjectItem(user) 发生错误\n");
        ret = -1;
        goto END;
    }
	strcpy(user, child1->valuestring);

    cJSON *child2 = cJSON_GetObjectItem(root, "pwd");
    if(NULL == child2)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "get_login_info():解析json包时 cJSON_GetObjectItem(pwd) 发生错误\n");
        ret = -1;
        goto END;
    }
	strcpy(pwd, child2->valuestring);

END:
    if(root != NULL)
    {
        cJSON_Delete(root);
        root = NULL;
    }

    return ret;
}

//判断用户登陆情况
int check_user_pwd(char *user, char *pwd)
{
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "check_user_pwd():连接Mysql数据库，判断用户登录情况\n");
	int ret = 0;
	MYSQL *conn = NULL;
	char sql_cmd[SQL_MAX_LEN] = {0};

    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "check_user_pwd():连接Mysql数据库时 msql_conn() 发送错误\n");
        ret = -1;
    }
    mysql_query(conn, "set names utf8");

    //查找用户对应的密码
    sprintf(sql_cmd, "select password from user where name=\"%s\"", user);
    //执行sql语句，结果集保存在tmp
    char tmp[PWD_LEN] = {0};
    process_result_one(conn, sql_cmd, tmp);
    if(strcmp(tmp, pwd) == 0)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "check_user_pwd():查询Mysql的结果 %s-%s\n", user, tmp);

    mysql_close(conn);

    return ret;
}

//生成token字符串, 保存到redis数据库
int set_token(char *user, char *token)
{
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "set_token():生成token字符串，保存在Redis数据库\n");

	int ret = 0;
    redisContext* redis_conn = NULL;

    //连接redis数据库
    redis_conn = rop_connectdb_nopwd(redis_ip, redis_port);
    if (redis_conn == NULL)
    {
        LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "set_token():连接Redis数据库时 rop_connectdb_nopwd() 发生错误\n");
        ret = -1;
        goto END;
    }

    //产生4个1000以内的随机数
    int rand_num[4] = {0};
    int i = 0;

    //设置随机种子
    srand((unsigned int)time(NULL));
    for(i = 0; i < 4; ++i)
    {
        rand_num[i] = rand()%1000;//随机数
    }
    char tmp[1024] = {0};
    sprintf(tmp, "%s%d%d%d%d", user, rand_num[0], rand_num[1], rand_num[2], rand_num[3]);

    //加密
    char enc_tmp[1024*2] = {0};
    int enc_len = 0;
    ret = DesEnc((unsigned char *)tmp, strlen(tmp), (unsigned char *)enc_tmp, &enc_len);
    if(ret != 0)
    {
        ret = -1;
        goto END;
    }

    //base64编码
    char base64[1024*3] = {0};
    base64_encode((const unsigned char *)enc_tmp, enc_len, base64);

    //转为md5
    MD5_CTX md5;
    MD5Init(&md5);
    unsigned char decrypt[16];
    MD5Update(&md5, (unsigned char *)base64, strlen(base64) );
    MD5Final(&md5, decrypt);

    char str[100] = { 0 };
    for (i = 0; i < 16; i++)
    {
        sprintf(str, "%02x", decrypt[i]);
        strcat(token, str);
    }
    LOG(LOGIN_LOG_MODULE, LOGIN_LOG_PROC, "set_token():生成token字符串\n");

    // redis保存此字符串, 有效时间为24小时
    ret = rop_setex_string(redis_conn, user, 86400, token);

END:
    if(redis_conn != NULL)
    {
        rop_disconnect(redis_conn);
    }

    return ret;
}

//返回客户端情况
void return_login_status(char *status_num, char *token)
{
	char *out = NULL;
    cJSON *root = cJSON_CreateObject();  				//创建json项目
    cJSON_AddStringToObject(root, "code", status_num);	// {"code":"xxx"}
    cJSON_AddStringToObject(root, "token", token);		// {"token":"token"}
    out = cJSON_Print(root);							// cJSON to string(char *)

    cJSON_Delete(root);

    if(out != NULL)
    {
        printf(out);
        free(out);
    }
}
