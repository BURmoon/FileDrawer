#include "logininfoinstance.h"

LoginInfoInstance::Garbo LoginInfoInstance::tmp;
LoginInfoInstance* LoginInfoInstance::instance = new LoginInfoInstance;

LoginInfoInstance::LoginInfoInstance()
{

}
LoginInfoInstance::~LoginInfoInstance()
{

}
LoginInfoInstance::LoginInfoInstance(const LoginInfoInstance& )
{

}
LoginInfoInstance& LoginInfoInstance::operator=(const LoginInfoInstance&)
{
    return *this;
}

//获取唯一的实例
LoginInfoInstance *LoginInfoInstance::getInstance()
{
    return instance;
}

//释放堆区空间
void LoginInfoInstance::destroy()
{
    if(NULL != LoginInfoInstance::instance)
    {
        delete LoginInfoInstance::instance;
        LoginInfoInstance::instance = NULL;
    }
}

//设置登陆信息
void LoginInfoInstance::setLoginInfo(QString tmpUser, QString tmpIp, QString tmpPort, QString token)
{
    user = tmpUser;
    ip = tmpIp;
    port = tmpPort;
    this->token = token;
}

//获取登陆用户
QString LoginInfoInstance::getUser() const
{
    return user;
}

//获取服务器ip
QString LoginInfoInstance::getIp() const
{
    return ip;
}

//获取服务器端口
QString LoginInfoInstance::getPort() const
{
    return port;
}

//获取登陆token
QString LoginInfoInstance::getToken() const
{
    return token;
}

