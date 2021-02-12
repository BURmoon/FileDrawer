#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDebug>
#include <QMap>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QTextEdit>
#include <QTimer>
#include <QFileInfo>
#include "common.h"
#include "logininfoinstance.h"
#include "uploadtask.h"
#include "downloadtask.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //初始化UI布局
    void initUI();
    //初始化鼠标右键菜单
    void initMenu();
    //触发右键菜单
    void rightMenu(const QPoint &pos);
    //信号处理相关
    void managerSignals();

    //==========>消息框处理<==============
    //在消息框中添加信息
    void addInform(QString title, QString str);
    //清空消息框
    void clearInform();

    //==========>文件处理<==============
    //显示文件属性
    void getFileProperty();
    //删除文件
    void delFile();

    //==========>文件item展示<==============
    // 清空文件列表
    void clearFileList();
    // 清空所有item项目
    void clearItems();
    // 文件item展示
    void refreshFileItems();

    //==========>显示用户的文件列表<==============
    //显示用户的文件列表
    void refreshFiles();
    //设置请求文件数json包
    QByteArray setGetCountJson(QString user, QString token);
    //得到服务器json文件
    QStringList getCountStatus(QByteArray json);
    //设置请求文件下载json包
    QByteArray setFilesListJson(QString user, QString token, int start, int count);
    //解析文件列表json信息，存放在文件列表中
    void getFileJsonInfo(QByteArray data);
    //获取用户文件列表
    void getUserFilesList();

    //==========>上传文件处理<==============
    //添加需要上传的文件到上传任务列表
    void addUploadFiles();
    //设置md5信息的json包
    QByteArray setMd5Json(QString user, QString token, QString md5, QString fileName);
    //上传文件处理，取出上传任务列表的队首任务，上传完后，再取下一个任务
    void uploadFilesAction();
    //上传真正的文件内容，不能秒传的前提下
    void uploadFile(UploadFileInfo *info);
    //显示上传文件列表
    void refreshUploadFiles();

    //==========>下载文件处理<==============
    //添加需要下载的文件到下载任务列表
    void addDownloadFiles();
    //下载文件处理，取出下载任务列表的队首任务，下载完后，再取下一个任务
    void downloadFilesAction();
    //下载文件pv字段处理
    void dealFilePv(QString md5, QString filename);
    //设置json包
    QByteArray setDealFileJson(QString user, QString token, QString md5, QString filename);
    //显示下载文件列表
    void refreshDownloadFiles();

    //清除上传下载任务
    void clearAllTask();
    //定时检查处理任务队列中的任务
    void checkTaskList();

    //显示主窗口
    void showMainWindow();

    //获取文件所对应的ContentType
    QString getContentType(QString str);

signals:
    void loginAgainSignal();
    void switchUser();

private:
    Ui::MainWindow *ui;

    Common m_cm;
    QNetworkAccessManager* m_manager;
    QMap<QString, QString> m_map;

    QList<FileInfo *> m_fileList;   //文件列表
    long m_userFilesCount;          //用户文件数目
    int m_start;                    //文件位置起点
    int m_count;                    //每次请求文件个数

    QListWidgetItem* upItem;        //当前正在上传的任务Item
    QListWidgetItem* downItem;      //当前正在下载的任务Item

    QMenu   *m_menuFile;            //菜单1
    QAction *m_propertyAction;      //属性
    QAction *m_downloadAction;      //下载
    QAction *m_delAction;           //删除
    QMenu   *m_menuEmpty;           //菜单2
    QAction *m_refreshAction;       //刷新
    QAction *m_uploadAction;        //上传


    //定时器
    QTimer m_uploadFileTimer;       //定时检查上传队列是否有任务需要上传
    QTimer m_downloadTimer;         //定时检查下载队列是否有任务需要下载
};
#endif // MAINWINDOW_H
