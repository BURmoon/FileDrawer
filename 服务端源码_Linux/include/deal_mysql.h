/*
 * mysql数据库处理相关接口
 */

#ifndef _DEAL_MYSQL_H_
#define _DEAL_MYSQL_H_

#include <mysql.h> //数据库

#define SQL_MAX_LEN         (512)   //sql语句长度

/************************************************
TODO: 显示操作数据库出错时的错误信息
# conn: 连接数据库的句柄
# title: 错误提示信息
# return: 
************************************************/
void print_error(MYSQL *conn, const char *title);

/************************************************
TODO: 连接数据库
# user_name: 数据库用户
# passwd: 数据库密码
# db_name: 数据库名称
# return: 
		成功: 连接数据库的句柄
		失败: NULL
************************************************/
MYSQL* msql_conn(char *user_name, char* passwd, char *db_name);

/************************************************
TODO: 处理数据库查询结果，并打印
# conn: 连接数据库的句柄
# res_set: 数据库查询后的结果集
# return: 
************************************************/
void process_result_test(MYSQL *conn, MYSQL_RES *res_set);

/************************************************
TODO: 处理数据库查询结果，只处理一条记录
# conn: 连接数据库的句柄
# sql_cmd: 执行的sql语句
# buf: 保存结果集
# return: 
		成功并保存结果集		0
		没有结果集 			1
		有结果集但是没有保存 	2
		失败 				-1
************************************************/
int process_result_one(MYSQL *conn, char *sql_cmd, char *buf);

#endif
