/*
 * 读取配置文件信息
 */

#ifndef _CFG_H_
#define _CFG_H_

#define CFG_PATH    "/root/ServerStorage/conf/cfg.json" //配置文件路径

/************************************************
TODO: 从配置文件中得到相对应的参数
# profile: 配置文件路径
# tile: 配置文件title名称
# key: 待查找的参数
# value: 参数 key 得找的 value
# return: 
		成功: 0
		失败: -1
************************************************/
extern int get_cfg_value(const char *profile, char *tile, char *key, char *value);

//获取数据库用户名、用户密码、数据库标示等信息
extern int get_mysql_info(char *mysql_user, char *mysql_pwd, char *mysql_db);


#endif
