#ifndef UPLOADTASK_H
#define UPLOADTASK_H

#include <QFile>
#include <QFileInfo>
#include "common.h"

//上传文件信息
struct UploadFileInfo
{
    QString md5;        //文件md5码
    QFile *file;        //文件指针
    QString fileName;   //文件名字
    qint64 size;        //文件大小
    QString path;       //文件路径
    bool isUpload;      //是否已经在上传
    QListWidgetItem *item;  // listwidget 的 item
};

//上传任务列表类，单例模式，一个程序只能有一个上传任务列表
class UploadTask
{
public:
    static UploadTask *getInstance(); //保证唯一一个实例

    //追加上传文件到上传列表中
    int appendUploadList(QString path);

    bool isEmpty();     //判断上传队列释放为空
    bool isUpload();    //是否有文件正在上传
    int listSize();     //获取下载队列的大小
    QListWidgetItem* takeItem(int i);   //取出下载队列中第i个任务的Item

    //取出第0个上传任务，如果任务队列没有任务在上传，设置第0个任务上传
    UploadFileInfo * takeTask();
    //删除上传完成的任务
    void dealUploadTask();
    //清空上传列表
    void clearList();

private:
    UploadTask();    //构造函数为私有
    ~UploadTask();    //析构函数为私有

    //静态数据成员，类中声明，类外必须定义
    static UploadTask *instance;

    //它的唯一工作就是在析构函数中删除Singleton的实例
    class Garbo
    {
    public:
        ~Garbo()
        {
          if(NULL != UploadTask::instance)
          {
            UploadTask::instance->clearList();

            delete UploadTask::instance;
            UploadTask::instance = NULL;
          }
        }
    };

    //定义一个静态成员变量，程序结束时，系统会自动调用它的析构函数
    //static类的析构函数在main()退出后调用
    static Garbo temp; //静态数据成员，类中声明，类外定义

    QList<UploadFileInfo *> list; //上传任务列表(任务队列)
};

#endif // UPLOADTASK_H
