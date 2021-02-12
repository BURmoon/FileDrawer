#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStatusBar>
#include "mainwindow.h"
#include "common.h"
#include "logininfoinstance.h"


namespace Ui {
class login;
}

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

    //设置登陆用户信息的json包
    QByteArray setLoginJson(QString user, QString pwd);
    //设置注册用户信息的json包
    QByteArray setRegisterJson(QString userName, QString firstPwd);
    //解析服务端返回的json数据，状态码返回值为 "000"/"001"
    QStringList getLoginStatus(QByteArray json);

private slots:
    //注册界面的返回按钮
    void on_back_btn_clicked();
    //注册界面的确认按钮
    void on_register_btn_clicked();
    //登录界面的确认按钮
    void on_login_btn_clicked();

private:
    Ui::login *ui;

    //处理网络请求类对象
    QNetworkAccessManager* m_manager;
    //主窗口指针
    MainWindow* m_mainWin;
    Common m_cm;
};

#endif // LOGIN_H
