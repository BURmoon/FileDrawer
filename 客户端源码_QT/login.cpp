#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);

    //窗口初始化
    // 设置固定窗口大小
    this->setFixedSize(320,388);
    // 设置窗口标题
    this->setWindowTitle("CloudS");
    // 设置port编辑框的大小
    ui->log_port_server->setFixedSize(40, 20);
    ui->reg_port_server->setFixedSize(40, 20);
    // 设置当前显示的窗口
    ui->stackedWidget->setCurrentIndex(0);
    // 设置键盘焦点
    ui->log_usr->setFocus();
    // 设置密码显示的样式
    ui->log_pwd->setEchoMode(QLineEdit::Password);
    ui->reg_pwd->setEchoMode(QLineEdit::Password);
    ui->reg_surepwd->setEchoMode(QLineEdit::Password);

    //初始化网络请求（http）类
    m_manager = Common::getNetManager();
    m_mainWin = new MainWindow;

    //按钮的信号处理
    // 注册按钮 --> 切换到注册窗口
    connect(ui->log_register_btn, &QToolButton::clicked, [=]()
    {
        ui->stackedWidget->setCurrentWidget(ui->register_2);
        ui->reg_usr->setFocus();
    });

    //主窗口发送切换用户信号
    connect(m_mainWin, &MainWindow::switchUser, [=]()
    {
        m_mainWin->hide();
        this->show();
    });

//测试用
#if 1
    ui->log_address_server->setText("81.70.28.177");
    ui->log_port_server->setText("80");
    ui->log_usr->setText("test");
    ui->log_pwd->setText("test");
#endif


}

login::~login()
{
    delete ui;
}

/************************************************
TODO: 将usr和pwd打包成Json格式，用于登录使用
# usr: 账户
# pwd: 密码
# return:
        QByteArray格式的Json数据包
************************************************/
QByteArray login::setLoginJson(QString user, QString pwd)
{
    QMap<QString, QVariant> login;
    login.insert("user", user);
    login.insert("pwd", pwd);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(login);
    if ( jsonDocument.isNull() )
    {
        //qDebug() << " login: jsonDocument.isNull() ";
        return "";
    }

    return jsonDocument.toJson();
}

/************************************************
TODO: 将userName和firstPwd打包成Json格式，用于注册使用
# userName： 账户
# firstPwd： 密码
# return：
        QByteArray格式的Json数据包
************************************************/
QByteArray login::setRegisterJson(QString userName, QString firstPwd)
{
    QMap<QString, QVariant> reg;
    reg.insert("userName", userName);
    reg.insert("firstPwd", firstPwd);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(reg);
    if ( jsonDocument.isNull() )
    {
        //qDebug() << " reg: jsonDocument.isNull() ";
        return "";
    }

    return jsonDocument.toJson();
}

/************************************************
TODO: 从json数据中解析出服务端回写的code和token
# json: 服务端回发的Json数据包
# return:
    QStringList列表，包含 状态码code 以及 登录token
************************************************/
QStringList login::getLoginStatus(QByteArray json)
{
    QJsonParseError error;
    QStringList list;

    //将源数据json转化为JsonDocument
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if(error.error == QJsonParseError::NoError)
    {
        if (doc.isNull() || doc.isEmpty())
        {
            //qDebug() << "doc.isNull() || doc.isEmpty()";
            return list;
        }

        if(doc.isObject())
        {
            //获取Json对象
            QJsonObject obj = doc.object();
            //qDebug() << "服务器返回数据：" << obj;
            //状态码
            list.append(obj.value("code").toString());
            //登陆token
            list.append(obj.value("token").toString());
        }
    }
    else
    {
        //qDebug() << "err = " << error.errorString();
    }

    return list;
}

/************************************************
TODO: 清空当前数据，并返回到登录界面
# 激活注册页面的返回按钮
************************************************/
void login::on_back_btn_clicked()
{
    if(ui->stackedWidget->currentWidget() == ui->register_2)
    {
        //清空数据
        ui->reg_address_server->clear();
        ui->reg_port_server->clear();
        ui->reg_usr->clear();
        ui->reg_pwd->clear();
        ui->reg_surepwd->clear();
        //切换到登录窗口
        ui->stackedWidget->setCurrentWidget(ui->login_2);
        ui->log_usr->setFocus();
    }

}

/************************************************
TODO: 向服务端发送注册请求，若注册成功则切换到登录界面
# 激活注册页面的确认按钮
************************************************/
void login::on_register_btn_clicked()
{
    qDebug() << "向服务端发送注册请求";
    //从控件中取出相应注册数据
    QString userName = ui->reg_usr->text();
    QString firstPwd = ui->reg_pwd->text();
    QString surePwd = ui->reg_surepwd->text();
    QString address = ui->log_address_server->text();
    QString port = ui->log_port_server->text();

    //密码校验
    if(surePwd != firstPwd)
    {
        QMessageBox::warning(this, "警告", "两次输入的密码不匹配, 请重新输入");
        ui->reg_pwd->clear();
        ui->reg_surepwd->clear();
        ui->reg_pwd->setFocus();
        return;
    }

    //设置注册信息json包
    // 注册协议：{ userName:xxxx, firstPwd:xxx }
    QByteArray array = setRegisterJson(userName, firstPwd);
    //设置连接服务端要发送的url
    QNetworkRequest request;
    QString url = QString("http://%1:%2/reg").arg(address).arg(port);
    request.setUrl(QUrl(url));
    //设置请求头信息
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(array.size()));

    //向服务器发送post请求
    QNetworkReply* reply = m_manager->post(request, array);

    //判断请求是否被成功处理
    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        /*  server端返回的注册json数据格式
            成功:         {"code":"001"}
            用户已存在:    {"code":"011"}
            失败:         {"code":"012"}  */
        //读取服务端回写的数据
        QByteArray json = reply->readAll();

        qDebug() << QString("服务端回写 %1").arg(m_cm.getCode(json));
        //判断状态码
        if(m_cm.getCode(json) == "001")
        {
            //注册成功
            QMessageBox::information(this, "注册成功", "注册成功，请登录");

            //清空编辑框内的内容
            ui->reg_usr->clear();
            ui->reg_pwd->clear();
            ui->reg_surepwd->clear();
            ui->log_address_server->clear();
            ui->log_port_server->clear();

            //设置登陆窗口的登陆信息
            ui->log_address_server->setText(address);
            ui->log_port_server->setText(port);
            ui->log_usr->setText(userName);
            ui->log_pwd->setText(firstPwd);

            //切换到登录界面
            ui->stackedWidget->setCurrentWidget(ui->login_2);
        }
        else if("011" == m_cm.getCode(json))
        {
            //用户已存在
            QMessageBox::warning(this, "注册失败", QString("[%1]该用户已经存在！").arg(userName));
        }
        else if("012" == m_cm.getCode(json))
        {
            //注册失败
            QMessageBox::warning(this, "注册失败", "注册失败！");
        }

        //释放资源
        delete reply;
    });
}

/************************************************
TODO: 向服务端发送登录请求，若登录成功则
      将登录信息保存在 LoginInfoInstance单例类 中
# 激活登录页面的确认按钮
************************************************/
void login::on_login_btn_clicked()
{
    //从控件中取出相应的登录信息
    QString user = ui->log_usr->text();
    QString pwd = ui->log_pwd->text();
    QString address = ui->log_address_server->text();
    QString port = ui->log_port_server->text();

    //设置登陆信息json包，密码经过md5加密
    // 登陆协议：{ user:xxxx, pwd:xxx }
    //QByteArray array = setLoginJson(user, m_cm.getStrMd5(pwd));
    QByteArray array = setLoginJson(user, pwd);
    //设置用于登录的url
    QNetworkRequest request;
    QString url = QString("http://%1:%2/login").arg(address).arg(port);
    request.setUrl(QUrl(url));
    //设置请求头信息
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(array.size()));

    //向服务器发送post请求
    QNetworkReply* reply = m_manager->post(request, array);

    //接收服务器发回的http响应消息
    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << reply->errorString();
            reply->deleteLater();   //释放资源
            return;
        }

        /*  服务器回写的登陆json数据格式：
            成功：{"code":"000"，"token": "xxx"}
            失败：{"code":"010"，"token": "fail"}  */

        //读取服务端回写的数据
        QByteArray json = reply->readAll();
        QStringList tmpList = getLoginStatus(json);
        qDebug() << QString("服务端回写 %1").arg(m_cm.getCode(json));
        if(tmpList.at(0) == "000")
        {
            qDebug() << "登陆成功";

            //通过单例类来保存登陆信息
            LoginInfoInstance *p = LoginInfoInstance::getInstance(); //获取单例
            //p->setLoginInfo(user, address, port, tmpList.at(1));
            p->setLoginInfo(user, address, port, tmpList.at(1));
            qDebug() << QString("%1-%2-%3-%4").arg(p->getIp()).arg(p->getPort()).arg(p->getUser()).arg(p->getToken());
            //隐藏当前窗口
            this->hide();
            //主界面窗口显示
            m_mainWin->showMainWindow();
        }
        else
        {
            QMessageBox::warning(this, "登录失败", "用户名或密码不正确！");
        }

        reply->deleteLater();   //释放资源
    });
}
