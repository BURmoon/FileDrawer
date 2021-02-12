#ifndef COMMON_H
#define COMMON_H

#include <QFile>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QListWidgetItem>
#include <QNetworkAccessManager>

//文件信息
struct FileInfo
{
    QString md5;            // 文件md5码
    QString filename;       // 文件名字
    QString user;           // 用户
    QString time;           // 上传时间
    QString url;            // url
    QString type;           // 文件类型
    qint64 size;            // 文件大小
    int pv;                 // 下载量
    QListWidgetItem *item;  // listwidget 的 item
};
//传输状态
enum TransferStatus{Download, Uplaod};

class Common : public QObject
{
public:
    Common(QObject* parent = 0);
    ~Common();

    //得到http通信类对象
    static QNetworkAccessManager* getNetManager();

    //得到服务器回复的状态码， 返回值为 "000" 或 "001"
    QString getCode(QByteArray json);

    //获取某个文件的md5码
    QString getFileMd5(QString filePath);
    // 将某个字符串加密成md5码
    QString getStrMd5(QString str = "");

private:
    //http类
    static QNetworkAccessManager *m_netManager;
};

#endif // COMMON_H
