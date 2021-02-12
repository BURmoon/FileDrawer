#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "cJSON.h"

//从配置文件中得到相对应的参数
int get_cfg_value(const char *profile, char *title, char *key, char *value)
{
    if(profile == NULL || title == NULL || key == NULL || value == NULL)
    {
        return -1;
    }

    int ret = 0;
    char *buf = NULL;
    FILE *fp = NULL;

    //只读方式打开文件
    fp = fopen(profile, "rb");
    if(fp == NULL) //打开失败
    {
        perror("fopen");
        ret = -1;
        goto END;
    }

    //光标移动到末尾
    fseek(fp, 0, SEEK_END);
    //获取文件大小
    long size = ftell(fp);
    //光标移动到开头
    fseek(fp, 0, SEEK_SET);

    buf = (char *)calloc(1, size+1); //动态分配空间
    if(buf == NULL)
    {
        perror("calloc");
        ret = -1;
        goto END;
    }

    //读取文件内容
    fread(buf, 1, size, fp);

    //解析一个json字符串为cJSON对象
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root)
    {
        ret = -1;
        goto END;
    }

    //返回 title 对应的json对象
    cJSON * father = cJSON_GetObjectItem(root, title);
    if(NULL == father)
    {
        ret = -1;
        goto END;
    }
    //返回 key 对应的json数据
    cJSON * son = cJSON_GetObjectItem(father, key);
    if(NULL == son)
    {
        ret = -1;
        goto END;
    }
    //拷贝内容
    strcpy(value, son->valuestring);

    cJSON_Delete(root);//删除json对象

END:
    if(fp != NULL)
    {
        fclose(fp);
    }

    if(buf != NULL)
    {
        free(buf);
    }

    return ret;
}


//获取数据库用户名、用户密码、数据库标示等信息
int get_mysql_info(char *mysql_user, char *mysql_pwd, char *mysql_db)
{
    if( -1 == get_cfg_value(CFG_PATH, "mysql", "user", mysql_user) )
    {
        return -1;
    }

    if( -1 == get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd) )
    {
        return -1;
    }

    if( -1 == get_cfg_value(CFG_PATH, "mysql", "database", mysql_db) )
    {
        return -1;
    }

    return 0;
}
