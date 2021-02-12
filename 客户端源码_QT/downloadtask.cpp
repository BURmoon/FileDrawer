#include "downloadtask.h"


DownloadTask * DownloadTask::instance = new DownloadTask;
DownloadTask::Garbo DownloadTask::temp;

DownloadTask::DownloadTask()
{

}
DownloadTask::~DownloadTask()
{

}
DownloadTask * DownloadTask::getInstance()
{
    return instance;
}

/************************************************
TODO: 清空下载列表
************************************************/
void DownloadTask::clearList()
{
    int n = list.size();
    for(int i = 0; i < n; ++i)
    {
        DownloadInfo *tmp = list.takeFirst();
        delete tmp;
    }
}

/************************************************
TODO: 判断下载队列是否为空
************************************************/
bool DownloadTask::isEmpty()
{
    return list.isEmpty();
}

/************************************************
TODO: 判断是否有文件正在下载
************************************************/
bool DownloadTask::isDownload()
{
    //遍历队列
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->isDownload == true) //说明有下载任务，不能添加新任务
        {
            return true;
        }
    }

    return false;
}

/************************************************
TODO: 获取下载队列的大小
************************************************/
int DownloadTask::listSize()
{
    return list.size();
}

/************************************************
TODO: 取出下载队列中第i个任务的Item
************************************************/
QListWidgetItem *DownloadTask::takeItem(int i)
{
    DownloadInfo *tmp = list.at(i);
    QListWidgetItem *item = tmp->item;

    return item;
}

/************************************************
TODO: 取出第0个下载任务
      如果任务队列没有任务在下载，设置第0个任务下载
************************************************/
DownloadInfo * DownloadTask::takeTask()
{
    if(isEmpty())
    {
        return NULL;
    }

    list.at(0)->isDownload = true; //标志为在下载
    return list.at(0);
}

/************************************************
TODO: 删除下载完成的任务
************************************************/
void DownloadTask::dealDownloadTask()
{
    //遍历队列
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->isDownload == true) //说明有下载任务
        {
            //移除此文件，因为已经上传完成了
            DownloadInfo *tmp = list.takeAt(i);

            QFile *file = tmp->file;
            file->close();  //关闭文件
            delete file;    //释放空间

            delete tmp; //释放空间
            return;
        }
    }
}

/************************************************
TODO: 追加任务到下载队列
# info： 下载文件信息
# filePathName：文件保存路径
# return
    0: 文件下载成功
    -1: 下载的文件已经在下载队列中
    -2: 打开文件失败
************************************************/
int DownloadTask::appendDownloadList( FileInfo *info, QString filePathName)
{
    //遍历查看一下，下载的文件是否已经在下载队列中
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->user == info->user && list.at(i)->filename == info->filename)
        {
            //cout << info->filename << " 已经在下载队列中 ";
            return -1;
        }
    }

    QFile *file = new QFile(filePathName); //文件指针分配空间

    if(!file->open(QIODevice::WriteOnly))
    {
        //如果打开文件失败，则删除 file，并使 file 指针为 NULL，然后返回
        delete file;
        file = NULL;
        return -2;
    }

    //动态创建节点
    DownloadInfo *tmp = new DownloadInfo;
    tmp->user = info->user;   //用户
    tmp->file = file;         //文件指针
    tmp->filename = info->filename; //文件名字
    tmp->md5 = info->md5;           //文件md5
    tmp->url = info->url;           //下载网址
    tmp->isDownload = false;        //没有在下载
    tmp->item = new QListWidgetItem(tmp->filename); //文件item

    //插入节点
    list.append(tmp);


    return 0;
}

