/*
 * 注册事件相关的CGI程序
 */

#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "util_cgi.h"
#include "deal_mysql.h"
#include "cfg.h"
#include "cJSON.h"
#include "make_log.h"


#define REG_LOG_MODULE       "cgi"
#define REG_LOG_PROC         "reg"

//mysql 数据库配置信息 用户名， 密码， 数据库名称
//static char mysql_user[128] = {0};
//static char mysql_pwd[128] = {0};
//static char mysql_db[128] = {0};
static char mysql_user[128] = {"root"};
static char mysql_pwd[128] = {"root"};
static char mysql_db[128] = {"CloudS"};

//读取配置信息
void read_cfg();
//注册用户，功返回0，失败返回-1, 用户已存在返回-2
int user_regiser(char *reg_buf);
//解析用户注册信息的json包
int get_reg_info(char *reg_buf, char *user, char *pwd);

int main()
{
	//read_cfg();

    LOG(REG_LOG_MODULE, REG_LOG_PROC, "main():执行注册事件的CGI程序\n");

	//阻塞等待连接
	while(FCGI_Accept() >= 0)
	{
		int len;
		//获取环境变量 CONTENT_LENGTH 的值
		char* contentLength = getenv("CONTENT_LENGTH");

		printf("Content-type: text/html\r\n\r\n");

		//获取消息实体的长度
		if(contentLength == NULL)
		{
			len = 0;
		}
		else
		{
			len = atoi(contentLength);
		}

#if 0   //测试用
        if (len <= 0)   //没有登陆用户信息
        {
            printf("No data from standard input\n");
        }
        else
        {
            fprintf(stderr, "用于进行注册事件的测试\n");
            char *out = return_status("001");
            if(out != NULL)
            {
                printf(out);
                free(out);
            }
        }

#else   //真正的注册程序
		if (len <= 0)	//没有登陆用户信息
        {
            printf("No data from standard input\n");
            LOG(REG_LOG_MODULE, REG_LOG_PROC, "main():没有登陆用户信息\n");
        }
        else	//获取登录信息
        {
			char buf[4*1024] = {0};
            int ret = 0;
            char *out = NULL;

			//从服务器读取内容
            ret = fread(buf, 1, len, stdin);
            if(ret == 0)
            {
                LOG(REG_LOG_MODULE, REG_LOG_PROC, "main():读取用户登录信息失败，fread()返回值为 0");
            	continue;
            }

            //进行用户注册
            ret = user_register(buf);
            
            if(ret == 0)	//注册成功，返回001
            {
            	out = return_status("001");
                LOG(REG_LOG_MODULE, REG_LOG_PROC, "main():注册成功，返回001\n");
            }
            else if(ret == -1)	//注册失败，返回012
            {
                out = return_status("012");
                LOG(REG_LOG_MODULE, REG_LOG_PROC, "main():注册失败，返回012\n");
            }
            else if(ret == -2)	//用户已存在，返回011
            {
               out = return_status("011");
               LOG(REG_LOG_MODULE, REG_LOG_PROC, "main():用户已存在，返回011\n");
            }

            //向客户端返回信息
            if(out != NULL)
            {
            	printf(out);
            	free(out);
            }
        }
#endif
	}

	return 0;
}

//读取配置信息
void read_cfg()
{
    LOG(REG_LOG_MODULE, REG_LOG_PROC, "main() --> read_cfg()\n");

    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);
}

//注册用户，功返回0，失败返回-1, 用户已存在返回-2
int user_register(char *reg_buf)
{
    LOG(REG_LOG_MODULE, REG_LOG_PROC, "main() --> user_register()\n");

	int ret = 0;
    MYSQL *conn = NULL;

    //获取注册用户的信息
    char user[128];
    char pwd[128];
    // 解析客户端发送的json包
    ret = get_reg_info(reg_buf, user, pwd);
    if(ret != 0)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "解析客户端发送的json包失败\n");
        goto END;
    }

    //连接数据库
    conn = msql_conn(mysql_user, mysql_pwd, mysql_db);
    if (conn == NULL)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "连接数据库失败\n");
        ret = -1;
        goto END;
    }
    //设置数据库编码，处理中文编码问题
    mysql_query(conn, "set names utf8");
    //查询该用户是否存在
    char sql_cmd[SQL_MAX_LEN] = {0};
    sprintf(sql_cmd, "select * from user where name = '%s'", user);
    int sql_ret = process_result_one(conn, sql_cmd, NULL);
    if(sql_ret == 2)	//如果用户存在
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "解析MySQL结果集，用户已存在\n");
        ret = -2;
        goto END;
    }

    //进行注册，将注册信息放入数据库
    sprintf(sql_cmd, "insert into user (name, password) values ('%s', '%s')", user, pwd);
    if(mysql_query(conn, sql_cmd) != 0)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "解析MySQL结果集，用户注册失败\n");
        ret = -1;
        goto END;
    }
	sprintf(sql_cmd, "insert into user_file_count (user, count) values ('%s', '%d')", user, 0);
    if(mysql_query(conn, sql_cmd) != 0)
    {
        LOG(REG_LOG_MODULE, REG_LOG_PROC, "解析MySQL结果集，用户注册失败\n");
        ret = -1;
        goto END;
    }

END:
    if(conn != NULL)
    {
        mysql_close(conn); //断开数据库连接
    }

    return ret;   
}

//解析用户注册信息的json包
int get_reg_info(char *reg_buf, char *user, char *pwd)
{
    LOG(REG_LOG_MODULE, REG_LOG_PROC, "main() --> get_reg_info()\n");
    
	int ret = 0;

	//解析json包
    // 将一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(reg_buf);
    if(NULL == root)
    {
        ret = -1;
        goto END;
    }

    //返回指定字符串对应的json对象：{ userName:xxx , firstPwd:xxx }
    // 用户
    cJSON *child1 = cJSON_GetObjectItem(root, "userName");
    if(NULL == child1)
    {
        ret = -1;
        goto END;
    }
    strcpy(user, child1->valuestring);
    // 密码
    cJSON *child2 = cJSON_GetObjectItem(root, "firstPwd");
    if(NULL == child2)
    {
        ret = -1;
        goto END;
    }
    strcpy(pwd, child2->valuestring);

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}
