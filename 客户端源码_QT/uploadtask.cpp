#include "uploadtask.h"

//静态数据成员，类中声明，类外定义
UploadTask* UploadTask::instance = new UploadTask;
UploadTask::Garbo UploadTask::temp;

UploadTask::UploadTask()
{

}
UploadTask::~UploadTask()
{

}

UploadTask * UploadTask::getInstance()
{
    return instance;
}

/************************************************
TODO: 追加上传文件到上传列表中
# path： 上传文件路径
# return
     0: 成功
    -1: 上传的文件已经在上传队列中
    -2: 打开文件失败
************************************************/
int UploadTask::appendUploadList(QString path)
{
    qint64 size = QFileInfo(path).size();

    //遍历查看一下，下载的文件是否已经在上传队列中
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->path == path)
        {
            //cout << QFileInfo( path ).fileName() << " 已经在上传队列中 ";
            return -1;
        }
    }

    //文件指针分配空间
    QFile* file = new QFile(path);

    if(!file->open(QIODevice::ReadOnly))
    {
        //如果打开文件失败，则删除 file，并使 file 指针为 0，然后返回
        delete file;
        file = NULL;
        return -2;
    }

    //获取文件属性信息
    QFileInfo info(path);
    // 动态创建节点
    Common mc;
    UploadFileInfo *tmp = new UploadFileInfo;
    tmp->md5 = mc.getFileMd5(path);     //获取文件的md5码("common.h")
    tmp->file = file;                   //文件指针
    tmp->fileName = info.fileName();    //文件名字
    tmp->size = size;                   //文件大小
    tmp->path = path;                   //文件路径
    tmp->isUpload = false;              //当前文件没有被上传
    tmp->item = new QListWidgetItem(tmp->fileName); //文件item
    // 插入节点
    list.append(tmp);

    return 0;
}

/************************************************
TODO: 判断上传队列释放为空
************************************************/
bool UploadTask::isEmpty()
{
    return list.isEmpty();
}

/************************************************
TODO: 判断是否有文件正在上传
************************************************/
bool UploadTask::isUpload()
{
    //遍历队列
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->isUpload == true) //说明有上传任务，不能添加新任务
        {
            return true;
        }
    }

    return false;
}

/************************************************
TODO: 获取下载队列的大小
************************************************/
int UploadTask::listSize()
{
    return list.size();
}

/************************************************
TODO: 取出下载队列中第i个任务的Item
************************************************/
QListWidgetItem *UploadTask::takeItem(int i)
{
    UploadFileInfo *tmp = list.at(i);
    QListWidgetItem *item = tmp->item;

    return item;
}

/************************************************
TODO: 取出第0个上传任务
      如果任务队列没有任务在上传，设置第0个任务上传
************************************************/
UploadFileInfo *UploadTask::takeTask()
{
    //取出第一个任务
    UploadFileInfo *tmp = list.at(0);
    list.at(0)->isUpload = true; //标志位，设置此文件在上传

    return tmp;
}

/************************************************
TODO: 删除上传完成的任务
************************************************/
void UploadTask::dealUploadTask()
{
    //遍历队列
    for(int i = 0; i != list.size(); ++i)
    {
        if( list.at(i)->isUpload == true) //说明有下载任务
        {
            //移除此文件，因为已经上传完成了
            UploadFileInfo *tmp = list.takeAt(i);

            //关闭打开的文件指针
            QFile *file = tmp->file;
            file->close();
            delete file;

            delete tmp; //释放空间

            return;
        }
    }
}

/************************************************
TODO: 清空上传列表
************************************************/
void UploadTask::clearList()
{
    int n = list.size();
    for(int i = 0; i < n; ++i)
    {
        UploadFileInfo *tmp = list.takeFirst();
        delete tmp;
    }
}
