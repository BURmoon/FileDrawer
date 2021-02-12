#!/bin/bash

# 启动 tracker 和 storage服务
tracker_start()
{
    ps aux | grep fdfs_trackerd | grep -v grep > /dev/null
    if [ $? -eq 0 ];then
        echo "fdfs_trackerd 已经在运行中"
    else
        sudo fdfs_trackerd  /etc/fdfs/tracker.conf
        if [ $? -ne 0 ];then
            echo "tracker start is failed"
        else
            echo "tracker start is success"
        fi
    fi
}

storage_start()
{
    ps aux | grep fdfs_storaged | grep -v grep > /dev/null
    if [ $? -eq 0 ];then
        echo "fdfs_storaged 已经在运行中"
    else
        sudo fdfs_storaged  /etc/fdfs/storage.conf
        if [ $? -ne 0 ];then
            echo "storage start is failed"
        else
            echo "storage start is success"
        fi
    fi
}

case $1 in
    storage)
        storage_start
        ;;
    tracker)
        tracker_start
        ;;
    all)
        storage_start
        tracker_start
        ;;
    stop)
        sudo fdfs_trackerd /etc/fdfs/tracker.conf stop
        sudo fdfs_storaged /etc/fdfs/storage.conf stop
        ;;
    *)
        echo "do nothing"
esac
