#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QFile>
#include <QUrl>
#include "common.h"

//下载文件信息
struct DownloadInfo
{
    QFile *file;        //文件指针
    QString user;       //下载用户
    QString filename;   //文件名字
    QString md5;        //文件md5
    QUrl url;           //下载网址
    bool isDownload;    //是否已经在下载
    QListWidgetItem *item;  // listwidget 的 item
};



//下载任务列表类，单例模式，一个程序只能有一个下载任务列表
class DownloadTask
{
public:
    static DownloadTask *getInstance(); //保证唯一一个实例

    void clearList();   //清空上传列表
    bool isEmpty();     //判断上传队列是否为空
    bool isDownload();  //是否有文件正在下载
    int listSize();     //获取下载队列的大小
    QListWidgetItem* takeItem(int i);   //取出下载队列中第i个任务的Item
    DownloadInfo *takeTask();   //取出第0个下载任务，如果任务队列没有任务在下载，设置第0个任务下载
    void dealDownloadTask();    //删除下载完成的任务


    //追加任务到下载队列
    int appendDownloadList( FileInfo *info, QString filePathName);

private:
    DownloadTask();
    ~DownloadTask();

    //静态数据成员，类中声明，类外必须定义
    static DownloadTask *instance;

    //它的唯一工作就是在析构函数中删除Singleton的实例
    class Garbo
    {
    public:
        ~Garbo()
        {
          if(NULL != DownloadTask::instance)
          {
            DownloadTask::instance->clearList();//清空上传列表

            delete DownloadTask::instance;
            DownloadTask::instance = NULL;
          }
        }
    };

    //定义一个静态成员变量，程序结束时，系统会自动调用它的析构函数
    //static类的析构函数在main()退出后调用
    static Garbo temp; //静态数据成员，类中声明，类外定义

    QList <DownloadInfo *> list;    //下载的任务队列
};

#endif // DOWNLOADTASK_H
