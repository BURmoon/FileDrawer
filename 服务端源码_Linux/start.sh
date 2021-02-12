#!/bin/bash

# 拷贝nginx的配置文件nginx.conf到默认目录
echo ============= Copy nginx.conf =============
sudo mv /usr/local/nginx/conf/nginx.conf /usr/local/nginx/conf/nginx.conf.old
sudo cp ./conf/nginx.conf /usr/local/nginx/conf
echo
echo ============= fastdfs ==============
# 关闭已启动的 tracker 和 storage
./fastdfs.sh stop
# 启动 tracker 和 storage
./fastdfs.sh all
echo
echo ============= fastCGI ==============
# 重启所有的 cgi程序
./fcgi.sh
echo
echo ============= nginx ==============
# 关闭nginx
./nginx.sh stop
# 启动nginx
./nginx.sh start
echo
echo ============= redis ==============
# 关闭redis
./redis.sh stop
# 启动redis
./redis.sh start
