/*
 * 日志相关
 */
#ifndef  _MAKE_LOG_H_
#define  _MAKE_LOG_H

#include<pthread.h>
#include<stdio.h>
#include<stdarg.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>

//写入内容
int out_put_file(char *path, char *buf);
//创建目录
int make_path(char *path, char *module_name, char *proc_name);
//创建目录并写入内容
int dumpmsg_to_file(char *module_name, char *proc_name, const char *filename,
                        int line, const char *funcname, char *fmt, ...);
#ifndef _LOG
#define LOG(module_name, proc_name, x...) \
        do{ \
		dumpmsg_to_file(module_name, proc_name, __FILE__, __LINE__, __FUNCTION__, ##x);\
	}while(0)
#else
#define LOG(module_name, proc_name, x...)
#endif

extern pthread_mutex_t ca_log_lock;

#endif



