#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "cfg.h"
#include "util_cgi.h"
#include "redis_op.h"

//去掉字符串两边的空白字符
int trim_space(char *inbuf)
{

    int i = 0;
    int j = strlen(inbuf) - 1;
    int count = 0;

    char *str = inbuf;
    if (str == NULL ) 
    {
        return -1;
    }

    //从头部开始去除空白字符
    while (isspace(str[i]) && str[i] != '\0')   //isspace()函数 检查所传的字符是否是空白字符
	{
        i++;
    }
    //从尾部开始去除空白字符
    while (isspace(str[j]) && j > i) 
	{
        j--;
    }
    //实际字符数
    count = j - i + 1;
    strncpy(inbuf, str + i, count);

    inbuf[count] = '\0';

    return 0;
}

//在字符串full_data中查找字符串substr第一次出现的位置
char* memstr(char* full_data, int full_data_len, char* substr) 
{ 
    if (full_data == NULL || full_data_len <= 0 || substr == NULL) 
	{ 
        return NULL; 
    } 
    if (*substr == '\0')
	{ 
        return NULL; 
    } 

	//所匹配子串的长度
    int sublen = strlen(substr); 

    int i;
    char* cur = full_data; 
    //实际所需匹配子串的长度
    int last_possible = full_data_len - sublen + 1;
    for (i = 0; i < last_possible; i++) 
	{ 
        if (*cur == *substr) 
		{ 
            if (memcmp(cur, substr, sublen) == 0)   //memcmp()函数 把两字符串的前几个字节进行比较
			{ 
                //cur 等于 substr
                return cur; 
            } 
        }
		
        cur++; 
    } 

    return NULL; 
} 

//解析url获得所要的query部分参数
int query_parse_key_value(const char *query, const char *key, char *value, int *value_len_p)
{
    char *temp = NULL;
    char *end = NULL;
    int value_len =0;

    //找到是否有key
    temp = strstr(query, key);
    if (temp == NULL)
    {
        return -1;
    }

    //指向 key 后面的 = 号
    temp += strlen(key);
    //temp 指向 value 的首字符
    temp++;
    //end 指向 value 的尾字符
    end = temp;
    while ('\0' != *end && '#' != *end && '&' != *end )
    {
        end++;
    }

    //获取 value 的长度
    value_len = end-temp;

    strncpy(value, temp, value_len);
    value[value_len] ='\0';

    if (value_len_p != NULL)
    {
        *value_len_p = value_len;
    }

    return 0;
}

//通过文件名file_name，得到文件后缀字符串
int get_file_suffix(const char *file_name, char *suffix)
{
    if (file_name == NULL)
    {
        return -1;
    }

    char *p = file_name;
    int len = 0;
    char *q=NULL;
    char *k= NULL;

    //使指针 q 指向字符串尾部
    q = p;
    while (*q != '\0')
    {
        q++;
    }

    //使指针 k 指向文件字符串的后缀
    k = q;
    while (*k != '.' && k != p)
    {
        k--;
    }

    //将文件后缀保存在 suffix 
    if (*k == '.')
    {
        k++;
        len = q - k;

        if (len != 0)
        {
            strncpy(suffix, k, len);
            suffix[len] = '\0';
        }
        else
        {
            strncpy(suffix, "null", 5);
        }
    }
    else
    {
        strncpy(suffix, "null", 5);
    }

    return 0;
}

//返回前端情况，若返回的指针不为空，则需要free
char * return_status(char *status_num)
{
    char *out = NULL;
    cJSON *root = cJSON_CreateObject(); //创建json项目
    cJSON_AddStringToObject(root, "code", status_num);  //{"code":"000"}
    out = cJSON_Print(root);

    cJSON_Delete(root);


    return out;
}

//验证登陆token，成功返回0，失败-1
int verify_token(char *user, char *token)
{
    int ret = 0;
    redisContext * redis_conn = NULL;
    char tmp_token[128] = {0};

    //redis 服务器ip、端口
    char redis_ip[30] = {0};
    char redis_port[10] = {0};

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);

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
