#!/bin/bash

case $1 in
    start)
        sudo /usr/local/nginx/sbin/nginx 
        if [ $? -eq 0 ];then
            echo "nginx start is success"
        else
            echo "nginx start is fail"
        fi
        ;;
    stop)
        sudo /usr/local/nginx/sbin/nginx -s quit
        if [ $? -eq 0 ];then
            echo "nginx stop is success"
        else
            echo "nginx stop is fail"
        fi
        ;;
    reload)
        sudo /usr/local/nginx/sbin/nginx -s reload
        if [ $? -eq 0 ];then
            echo "nginx reload is success"
        else
            echo "nginx reload is fail"
        fi
        ;;
    *)
        echo "do nothing"
        ;;
esac

