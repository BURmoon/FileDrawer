#include"make_log.h"

pthread_mutex_t ca_log_lock=PTHREAD_MUTEX_INITIALIZER;

int dumpmsg_to_file(char *module_name, char *proc_name, const char *filename,
                        int line, const char *funcname, char *fmt, ...)
{
        char mesg[4096]={0};
        char buf[4096]={0};
        char filepath[1024] = {0};
        time_t t=0;
        struct tm * now=NULL;                                                                                     
        va_list ap;                                                                                               

        time(&t);                                                                                                 
        now = localtime(&t);                                       
        va_start(ap, fmt);                                                                               
        vsprintf(mesg, fmt, ap);                                                                       
        va_end(ap);                        

        snprintf(buf, 4096, "[%04d-%02d-%02d %02d:%02d:%02d]--[%s:%d]--%s",
                                now -> tm_year + 1900, now -> tm_mon + 1,                                         
                                now -> tm_mday, now -> tm_hour, now -> tm_min, now -> tm_sec,                     
                                filename, line, mesg);                                     
		//printf("%s \n", mesg);
		//printf("%s \n", buf);
                           
        make_path(filepath, module_name, proc_name);
        
        pthread_mutex_lock(&ca_log_lock);
        out_put_file(filepath, buf);     
        pthread_mutex_unlock(&ca_log_lock);

        return 0;     
}

int make_path(char *path, char *module_name, char *proc_name)
{
    time_t t;
    struct tm *now = NULL;
    char log_dir[1024] = {"/root/ServerStorage/logs"};
    time(&t);
    now = localtime(&t);

    snprintf(path, 1024, "/root/ServerStorage/logs/%s_%04d%02d%02d_%s.log", 
        module_name, now -> tm_year + 1900, now -> tm_mon + 1, now -> tm_mday, proc_name);
    
    if(access(log_dir, 0) == -1) 
    {
        if(mkdir(log_dir, 0777) == -1) 
        {
            fprintf(stderr, "create %s failed!\n", log_dir);    
        }         
    } 

    return 0;
}

int out_put_file(char *path, char *buf)
{
    int fd;                                                                                                   
    fd = open(path, O_RDWR | O_CREAT | O_APPEND, 0777);

    if(write(fd, buf, strlen(buf)) != (int)strlen(buf))
    {                                      
        fprintf(stderr, "write error!\n");                           
        close(fd);                                                                                        
    } 
    else 
    {
        close(fd);
    }
    
    return 0;
}
