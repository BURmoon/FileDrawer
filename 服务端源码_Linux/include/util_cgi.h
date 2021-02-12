/*
 * cgi后台通用接口
 */

#ifndef _UTIL_CGI_H_
#define _UTIL_CGI_H_

#define FILE_NAME_LEN       (256)	//文件名字长度
#define TEMP_BUF_MAX_LEN    (512)	//临时缓冲区大小
#define FILE_URL_LEN        (512)   //文件所存放storage的host_name长度
#define HOST_NAME_LEN       (30)	//主机ip地址长度
#define USER_NAME_LEN       (128)	//用户名字长度
#define TOKEN_LEN           (128)	//登陆token长度
#define MD5_LEN             (256)   //文件md5长度
#define PWD_LEN             (256)	//密码长度
#define TIME_STRING_LEN     (25)    //时间戳长度
#define SUFFIX_LEN          (8)     //后缀名长度

/************************************************
TODO: 去掉字符串两边的空白字符
# inbuf: 需要修改的字符串
# return: 
		成功: 0
		失败: -1
************************************************/
int trim_space(char *inbuf);

/************************************************
TODO: 在字符串full_data中查找字符串substr第一次出现的位置
# full_data: 源字符串首地址
# full_data_len: 源字符串长度
# substr: 匹配字符串首地址
# return: 
		成功: 匹配字符串首地址
		失败: NULL
************************************************/
char* memstr(char* full_data, int full_data_len, char* substr);

/************************************************
TODO: 解析url获得所要的query部分参数
# query: URL地址中 "?" 后面的内容
# key: 查找参数的名称
# value: 查找参数所对应的值
# value_len_p: 查找参数所对应值的长度
# return: 
		成功: 0
		失败: -1
************************************************/
int query_parse_key_value(const char *query, const char *key, char *value, int *value_len_p);

/************************************************
TODO: 通过文件名 file_name 得到文件后缀字符串
# file_name: 文件名
# suffix: 文件后缀
# return: 
		成功: 匹配字符串首地址
		失败: NULL
************************************************/
int get_file_suffix(const char *file_name, char *suffix);

/************************************************
TODO: 将 status_num 打包成JSON字符串返还给前端
# status_num: 待打包的JSON数据
# return: 
		成功: 匹配字符串首地址，需要free
		失败: NULL
************************************************/
char * return_status(char *status_num);

/************************************************
TODO: 验证登陆token
# user: 待验证的用户名
# token: 待验证的token字符串
# return: 
		成功: 0
		失败: -1
************************************************/
int verify_token(char *user, char *token);


#endif
