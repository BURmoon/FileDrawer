#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化UI界面
    initUI();
    //初始化鼠标右键菜单
    initMenu();
    //初始化信号处理
    managerSignals();

    //http管理类对象
    m_manager = Common::getNetManager();

    addInform("OK", "初始化完成");
}

MainWindow::~MainWindow()
{
    delete ui;
}

/************************************************
TODO: 初始化UI界面
************************************************/
void MainWindow::initUI()
{
    //设置窗口大小
    this->setFixedSize(320,588);

    //初始化listWidget文件列表
    // 设置列表不能拖动
    ui->listWidget->setMovement(QListView::Static);
    // 设置无边框
    ui->listWidget->setFrameShape(QFrame::NoFrame);
    // listWidget右键菜单
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::rightMenu);

    //初始化textEdit消息框
    // 设置textEdit高度
    ui->textEdit->setFixedHeight(100);
    // 设置textEdit左对齐
    ui->textEdit->setAlignment(Qt::AlignLeft);
    // 设置textEdit只读
    ui->textEdit->setReadOnly(true);
    // 设置textEdit最大显示段数
    // 当前只显示100段，当多余100段后，将会自动删除前面的数据
    ui->textEdit->document()->setMaximumBlockCount(100);
}

/************************************************
TODO: 初始化鼠标右键菜单
************************************************/
void MainWindow::initMenu()
{
    m_menuFile = new QMenu(this);
    m_menuEmpty = new QMenu(this);

    //初始化菜单项
    m_propertyAction = new QAction("属性", this);
    m_downloadAction = new QAction("下载", this);
    m_delAction = new QAction("删除", this);
    m_refreshAction = new QAction("刷新", this);
    m_uploadAction = new QAction("上传", this);

    //将动作添加到菜单1
    m_menuFile->addAction(m_propertyAction);
    m_menuFile->addAction(m_downloadAction);
    m_menuFile->addAction(m_delAction);
    //m_menu->addSeparator();     //添加分割线
    //将动作添加到菜单2
    m_menuEmpty->addAction(m_refreshAction);
    m_menuEmpty->addAction(m_uploadAction);
    //m_menu->addSeparator();

    //菜单相关按钮的信号处理
    // 属性
    connect(m_propertyAction, &QAction::triggered, [=]()
    {
        //QListWidgetItem *item = ui->listWidget->currentItem();
        //添加文件属性信息打印到消息框中
        getFileProperty();
    });
    // 下载
    connect(m_downloadAction, &QAction::triggered, [=]()
    {
        //添加需要下载的文件到下载任务列表
        addDownloadFiles();
    });
    // 删除
    connect(m_delAction, &QAction::triggered, [=]()
    {
        //删除文件
        delFile();
    });
    // 刷新
    connect(m_refreshAction, &QAction::triggered, [=]()
    {
        //显示用户的文件列表
        refreshFiles();
    });
    // 上传
    connect(m_uploadAction, &QAction::triggered, [=]()
    {
        //添加需要上传的文件到上传任务列表
        addUploadFiles();
    });
}

/************************************************
TODO: 触发右键菜单
************************************************/
void MainWindow::rightMenu(const QPoint &pos)
{
    QListWidgetItem *item = ui->listWidget->itemAt(pos);
    //在鼠标点击的地方弹出菜单
    if(item == NULL)
    {
        m_menuEmpty->exec(QCursor::pos());
    }
    else
    {
        m_menuFile->exec(QCursor::pos());
    }

}

/************************************************
TODO: 对窗口的信号处理
************************************************/
void MainWindow::managerSignals()
{
    //我的文件
    connect(ui->MyFile, &QAction::triggered, [=]()
    {
        addInform("OK", QString("切换至用户文件列表"));
        //显示用户的文件列表
        refreshFiles();
    });

    //下载列表
    connect(ui->DownList, &QAction::triggered, [=]()
    {
        addInform("OK", "切换至下载文件列表");
        //显示下载文件列表
        refreshDownloadFiles();

    });

    //上传列表
    connect(ui->UpList, &QAction::triggered, [=]()
    {
        addInform("OK", "切换至上传文件列表");
        //显示上传文件列表
        refreshUploadFiles();
    });

    //切换用户
    connect(ui->SwitchUser, &QAction::triggered, [=]()
    {
        //发送信号，告诉登陆窗口，切换用户
        emit switchUser();

        //清空上一个用户的上传或下载任务
        clearAllTask();
        //清空上一个用户的文件显示信息
        clearFileList();
        clearItems();
    });

}

/************************************************
TODO: 在消息框中添加相关信息
# title: 通知的标题
# str: 通知的内容
************************************************/
void MainWindow::addInform(QString title, QString str)
{
#if 1   //测试用
    QString m_title = "[" + title + "] " + str;
    ui->textEdit->append(m_title);
#else
    if(title != "TEST")
    {
        QString m_title = "[" + title + "] " + str;
        ui->textEdit->append(m_title);
    }
#endif
}

/************************************************
TODO: 清空消息框
************************************************/
void MainWindow::clearInform()
{
    ui->textEdit->clear();
}

/************************************************
TODO: 显示文件属性信息
      将文件的 类型/大小/上传时间/下载量 显示在消息框中
************************************************/
void MainWindow::getFileProperty()
{
    addInform("TEST", "显示文件属性信息");
    //获取当前选中的item
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == NULL)
    {
        return;
    }

    for(int i = 0; i < m_fileList.size(); ++i)
    {
        if(m_fileList.at(i)->item == item)  
        {
            FileInfo *info = m_fileList.at(i);
            addInform("文件属性", info->filename);
            ui->textEdit->append(QString("文件类型: %1").arg(info->type));
            ui->textEdit->append(QString("文件大小: %1").arg(info->size));
            ui->textEdit->append(QString("上传时间: %1").arg(info->time));
            ui->textEdit->append(QString("下载量: %1").arg(info->pv));
            break;  //中断
        }
    }
}

/************************************************
TODO: 将文件从文件列表 m_fileList 中删除
************************************************/
void MainWindow::delFile()
{
    FileInfo *info = NULL;
    //获取当前选中的item
    int i = 0;
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == NULL)
    {
        return;
    }
    for(i = 0; i < m_fileList.size(); ++i)
    {
        if(m_fileList.at(i)->item == item)
        {
            info = m_fileList.at(i);
            break;
        }
    }

    addInform("TEST", QString("删除文件 %1").arg(info->filename));

    //设置请求对象
    QNetworkRequest request;
    //获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); //获取单例
    //127.0.0.1:80/dealfile?cmd=del
    QString url = QString("http://%1:%2/dealfile?cmd=del").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url)); //设置url
    //qt默认的请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QByteArray data = setDealFileJson( login->getUser(),  login->getToken(), info->md5, info->filename);
    //发送post请求
    QNetworkReply * reply = m_manager->post(request, data);
    if(reply == NULL)
    {
        addInform("FAIL", "文件删除失败");
        return;
    }

    //获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply, &QNetworkReply::finished, [=]()
    {
        if(reply->error() != QNetworkReply::NoError) //出现错误
        {
            addInform("TEST", reply->errorString());
            addInform("FAIL", "文件删除失败");
            reply->deleteLater(); //释放资源
            return;
        }

        //服务器返回用户的数据
        QByteArray array = reply->readAll();
        reply->deleteLater();

        /*
            删除文件：
                成功：{"code":"006"}
                失败：{"code":"019"}
        */
        addInform("TEST", QString("删除文件：服务端回复消息 %1").arg(m_cm.getCode(array)));

        if("006" == m_cm.getCode(array) )
        {
            addInform("OK", QString("%1 删除成功").arg(info->filename));
            //从文件列表中移除该文件，并且移除列表视图中的item
            ui->listWidget->removeItemWidget(item);
            delete item;
            m_fileList.removeAt(i);
            delete info;
        }
        else if("019" == m_cm.getCode(array))
        {
            addInform("FAIL", QString("%1 删除失败").arg(info->filename));
        }
        else if("111" == m_cm.getCode(array)) //token验证失败
        {
            addInform("FAIL", QString("账户异常，请重新登陆"));
            return;
        }
    });
}

/************************************************
TODO: 顺序清空当前的文件列表
************************************************/
void MainWindow::clearFileList()
{
    int n = m_fileList.size();
    for(int i = 0; i < n; ++i)
    {
        FileInfo *tmp = m_fileList.takeFirst();
        delete tmp;
    }
}

/************************************************
TODO: 清除当前的文件列表Item中的数据
************************************************/
void MainWindow::clearItems()
{
    int n = ui->listWidget->count();
    for(int i = 0; i < n; ++i)
    {
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        delete item;
    }
}

/************************************************
TODO: 将文件列表显示到列表Item中
************************************************/
void MainWindow::refreshFileItems()
{
    //清空当前item项目
    clearItems();

    //如果文件列表不为空，显示文件列表
    if(m_fileList.isEmpty() == false)
    {
        int n = m_fileList.size(); //元素个数
        addInform("OK", QString("当前用户存有 %1 个文件").arg(n));
        for(int i = 0; i < n; ++i)
        {
            FileInfo *tmp = m_fileList.at(i);
            QListWidgetItem *item = tmp->item;
            ui->listWidget->addItem(item);
        }
    }
    else
    {
        addInform("OK", QString("当前用户存有 0 个文件"));
    }
}

/************************************************
TODO: 刷新显示用户的文件列表
************************************************/
void MainWindow::refreshFiles()
{
    addInform("TEST", "获取用户文件数");
    //初始化用户文件数目
    m_userFilesCount = 0;

    //从服务端获取用户文件个数
    // 设置请求对象
    QNetworkRequest request;
    // 获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    // 设置url: 127.0.0.1:80/myfiles&cmd=count
    QString url = QString("http://%1:%2/myfiles?cmd=count").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url));
    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // 设置需要传输的数据
    QByteArray data = setGetCountJson(login->getUser(), login->getToken());
    QNetworkReply * reply = m_manager->post(request,data);
    if(reply == NULL)
    {
        addInform("FAIL", "获取文件失败");
        return;
    }

    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            addInform("TEST", reply->errorString());
            addInform("FAIL", "获取文件失败");
            reply->deleteLater();
            return;
        }
        //读取Json数据
        QByteArray array = reply->readAll();
        reply->deleteLater();

        addInform("TEST", QString("获取文件数：服务端回复消息 %1").arg(m_cm.getCode(array)));
        //若token验证失败
        if("111" == m_cm.getCode(array))
        {
            addInform("FAIL", "账户异常，请重新登陆");
            return;
        }
        //获取文件数失败
        if("013" == m_cm.getCode(array))
        {
            addInform("FAIL", "获取用户文件失败，请尝试刷新");
            return;
        }

        //处理Json数据
        QStringList list = getCountStatus(array);
        //获取用户文件的数目
        m_userFilesCount = list.at(1).toLong();
        addInform("TEST", QString("获取用户文件的数目 %1").arg(m_userFilesCount));
        //清空当前的文件列表
        clearFileList();
        //获取文件列表
        if(m_userFilesCount > 0)
        {
            //服务端有文件，获取用户文件列表
            m_start = 0;  //从0开始
            m_count = 10; //每次请求10个

            //获取新的文件列表信息
            getUserFilesList();
        }
        else //没有文件
        {
            //更新item
            refreshFileItems();
        }
    });
}

/************************************************
TODO: 将user和token打包成Json格式
************************************************/
QByteArray MainWindow::setGetCountJson(QString user, QString token)
{
    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("token", token);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if(jsonDocument.isNull())
    {
        addInform("TEST", "json is NULL of setGetCountJson()");
        return "";
    }

    return jsonDocument.toJson();
}

/************************************************
TODO: 从json数据中解析出服务端回写的token和num(文件个数)
************************************************/
QStringList MainWindow::getCountStatus(QByteArray json)
{
    QJsonParseError error;
    QStringList list;

    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if(error.error == QJsonParseError::NoError)
    {
        if(doc.isNull() || doc.isEmpty())
        {
            addInform("TEST", "json is Null/Empty of getCountStatus()");
            return list;
        }

        if(doc.isObject())
        {
            QJsonObject obj = doc.object();
            list.append(obj.value("code").toString());
            list.append(obj.value("num").toString());
        }
    }
    else
    {
        addInform("TEST","getCountStatus() is error");
    }

    return list;
}

/************************************************
TODO: 将user、token、start和count打包成Json格式
      向服务端请求count个文件的信息
************************************************/
QByteArray MainWindow::setFilesListJson(QString user, QString token, int start, int count)
{
    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("token", token);
    tmp.insert("start", start);
    tmp.insert("count", count);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if ( jsonDocument.isNull() )
    {
        addInform("TEST", "json is Null of setFilesListJson()");
        return "";
    }

    return jsonDocument.toJson();
}

/************************************************
TODO: 从json数据中解析出服务端回写的所有文件信息
      并保存在 文件列表m_fileList 中
************************************************/
void MainWindow::getFileJsonInfo(QByteArray data)
{
    QJsonParseError error;

    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error == QJsonParseError::NoError)
    {
        if (doc.isNull() || doc.isEmpty())
        {
            addInform("TEST", "json is Null/Empty of getFileJsonInfo()");
            return;
        }

        if(doc.isObject())
        {
            QJsonObject obj = doc.object();
            QJsonArray array = obj.value("files").toArray();

            int size = array.size();
            for(int i = 0; i < size; ++i)
            {
                QJsonObject tmp = array[i].toObject();
                FileInfo *info = new FileInfo;
                info->user = tmp.value("user").toString();  //用户
                info->md5 = tmp.value("md5").toString();    //文件md5
                info->time = tmp.value("time").toString();  //上传时间
                info->filename = tmp.value("fileName").toString();  //文件名字
                info->pv = tmp.value("pv").toInt();         //下载量
                info->url = tmp.value("url").toString();    //url
                info->size = tmp.value("size").toInt();     //文件大小，以字节为单位
                info->type = tmp.value("type").toString();  //文件后缀
                info->item = new QListWidgetItem(info->filename);

                //list添加节点
                m_fileList.append(info);
            }
        }
    }
    else
    {
        addInform("TEST", error.errorString());
    }
}

/************************************************
TODO: 从服务端获取所有文件信息，并解析到文件列表中
************************************************/
void MainWindow::getUserFilesList()
{
    //函数递归结束的条件
    if(m_userFilesCount <= 0)
    {
        refreshFileItems();
        return;
    }
    //若请求文件数大于现有文件数，则重新赋值请求文件数
    if(m_count > m_userFilesCount)
    {
        m_count = m_userFilesCount;
    }

    //创建请求对象
    QNetworkRequest request;
    //获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    //获取用户文件信息 127.0.0.1:80/myfiles?cmd=normal
    QString url = QString("http://%1:%2/myfiles?cmd=normal").arg(login->getIp()).arg(login->getPort());
    //设置url
    request.setUrl(QUrl(url));
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    //设置请求数据
    QByteArray data = setFilesListJson(login->getUser(), login->getToken(), m_start, m_count);
    //改变文件起点位置
    m_start += m_count;
    m_userFilesCount -= m_count;

    //发送post请求
    QNetworkReply * reply = m_manager->post(request, data);
    if(reply == NULL)
    {
        addInform("FAIL", "获取文件失败");
        return;
    }
    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            addInform("TEST", reply->errorString());
            addInform("FAIL", "获取文件失败");
            reply->deleteLater();
            return;
        }

        //服务器返回数据
        QByteArray array = reply->readAll();
        reply->deleteLater();

        //token验证失败
        if("111" == m_cm.getCode(array))
        {
            addInform("FAIL", "账户异常，请重新登陆");
            return;
        }

        //获取文件信息失败
        if("014" == m_cm.getCode(array))
        {
            addInform("FAIL", "获取用户文件失败，请尝试刷新");
            return;
        }

        //不是错误码，则处理文件列表json信息
        // 解析文件列表的json信息，存放在文件列表中
        getFileJsonInfo(array);
        // 继续从服务端获取文件列表
        getUserFilesList();
    });
}

/************************************************
TODO: 添加需要上传的文件到上传任务列表
************************************************/
void MainWindow::addUploadFiles()
{
    //获取上传列表实例
    UploadTask *uploadList = UploadTask::getInstance();
    if(uploadList == NULL)
    {
        return;
    }

    //获取文件path
    QStringList list = QFileDialog::getOpenFileNames();

    //将所选文件添加到上传列表
    for(int i = 0; i < list.size(); ++i)
    {
        QFileInfo info(list.at(i));
        int res = uploadList->appendUploadList(list.at(i));    
        if(res == -1)
        {
            addInform("UPLOAD", QString("%1 已存在于上传队列中").arg(info.fileName()));
        }
        else if(res == -2)
        {
            addInform("UPLOAD", QString("%1 文件打开失败").arg(info.fileName()));
        }
        addInform("UPLOAD", QString("%1 成功添加至上传队列中").arg(info.fileName()));
    }

    //处理上传文件
    uploadFilesAction();
}

/************************************************
TODO: 将user、token、md5和fileName打包成Json格式
************************************************/
QByteArray MainWindow::setMd5Json(QString user, QString token, QString md5, QString fileName)
{
    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("token", token);
    tmp.insert("md5", md5);
    tmp.insert("fileName", fileName);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if (jsonDocument.isNull())
    {
        addInform("TEST", "json is Null of setMd5Json()");
        return "";
    }

    return jsonDocument.toJson();
}

/************************************************
TODO: 将上传任务列表中的任务逐个获取，并将文件的MD5信息上传
      检查后台服务端是否有此文件
************************************************/
void MainWindow::uploadFilesAction()
{
    addInform("TEST", "MD5秒传文件");

    //获取上传列表实例
    UploadTask *uploadList = UploadTask::getInstance();
    if(uploadList == NULL)
    {
        return;
    }

    if(uploadList->isEmpty())
    {
        return;
    }

    //查看当前是否有上传任务，单任务上传，若有任务则取消上传
    if(uploadList->isUpload())
    {
        return;
    }

    //获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QNetworkRequest request;
    QString url = QString("http://%1:%2/md5").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    //获取首个上传任务
    UploadFileInfo *info = uploadList->takeTask();
    QByteArray array = setMd5Json(login->getUser(), login->getToken(), info->md5, info->fileName);
    QNetworkReply *reply = m_manager->post(request, array);
    if(reply == NULL)
    {
        addInform("UPLOAD", "上传文件失败");
        return;
    }

    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError) //有错误
        {
            addInform("TEST", reply->errorString());
            addInform("UPLOAD", "上传文件失败");
            reply->deleteLater();
            return;
        }

        QByteArray array = reply->readAll();
        reply->deleteLater();

        /*
            秒传文件：
                文件已存在：{"code":"015"}
                秒传成功：  {"code":"003"}
                秒传失败：  {"code":"016"}
                token验证失败：{"code":"111"}
        */
        addInform("TEST", QString("MD5秒传文件：服务端回复消息 %1").arg(m_cm.getCode(array)));
        if("003" == m_cm.getCode(array))
        {
            //秒传文件成功
            addInform("UPLOAD", QString("%1 文件上传成功").arg(info->fileName));
            //删除已经完成的上传任务
            uploadList->dealUploadTask();
        }
        else if("016" == m_cm.getCode(array) )
        {
            //后台服务端没有此文件，需要真正的文件上传
            uploadFile(info);
        }
        else if("015" == m_cm.getCode(array) )
        {
            //上传的文件已存在
            addInform("UPLOAD", QString("%1 文件已经存在").arg(info->fileName));
            //删除已经完成的上传任务
            uploadList->dealUploadTask();
        }
        else if("111" == m_cm.getCode(array))
        {
            //token验证失败
            addInform("FAIL", "账户异常，请重新登陆");
            return;
        }
    });
}

/************************************************
TODO: 秒传失败，上传真正的文件内容
************************************************/
void MainWindow::uploadFile(UploadFileInfo *info)
{
    addInform("TEST", "上传文件");

    //取出上传任务
    QFile *file = info->file;           //文件指针
    QString fileName = info->fileName;  //文件名字
    QString md5 = info->md5;            //文件md5码
    QString path = info->path;          //文件路径
    //获取上传文件的后缀
    QFileInfo qinfo(path);
    QString suffix = qinfo.suffix();

    //获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance();

    QHttpPart part;
    QString disp = QString("form-data; user=\"%1\"; filename=\"%2\"; md5=\"%3\"; size=%4")
            .arg(login->getUser()).arg(info->fileName).arg(info->md5).arg(info->size);
    part.setHeader(QNetworkRequest::ContentDispositionHeader, disp);
    QString type = getContentType(suffix);  //获取传输的文件对应的content-type
    part.setHeader(QNetworkRequest::ContentTypeHeader, type);
    part.setBodyDevice(file);
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);
    multiPart->append(part);

    QNetworkRequest request;
    QString url = QString("http://%1:%2/upload").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");

    //发送post请求
    QNetworkReply * reply = m_manager->post(request, multiPart);
    if(reply == NULL)
    {
        addInform("UPLOAD", "上传文件失败");
        return;
    }

    //获取上传进度
    connect(reply, &QNetworkReply::uploadProgress, [=](qint64 bytesRead, qint64 totalBytes)
    {
        if(totalBytes != 0)
        {
            int ret = bytesRead/totalBytes;
            if(ret == 0.5)
            {
                addInform("upload", QString("%1 已经上传 50%").arg(fileName));
            }
            else if(ret == 0.9)
            {
                addInform("upload", QString("%1 已经上传 90%").arg(fileName));
            }
        }
    });

    connect(reply, &QNetworkReply::finished, [=]()
    {
        //获取上传列表实例
        UploadTask *uploadList = UploadTask::getInstance();
        if(uploadList == NULL)
        {
            return;
        }

        if (reply->error() != QNetworkReply::NoError) //有错误
        {
            addInform("TEST", reply->errorString());
            addInform("UPLOAD", "上传文件失败");
            reply->deleteLater(); //释放资源
            uploadList->dealUploadTask(); //删除已经完成的上传任务
            return;
        }

        QByteArray array = reply->readAll();
        reply->deleteLater();
        multiPart->deleteLater();

        /*
            上传文件：
                成功：{"code":"004"}
                失败：{"code":"017"}
        */
        addInform("TEST", QString("上传文件：服务端回复消息 %1").arg(m_cm.getCode(array)));
        if("004" == m_cm.getCode(array) )
        {
            //上传完成
            addInform("UPLOAD", QString("%1 文件上传成功").arg(info->fileName));
        }
        else if("017" == m_cm.getCode(array))
        {
            //上传失败
            addInform("UPLOAD", QString("%1 文件上传失败").arg(info->fileName));
        }

        uploadList->dealUploadTask(); //删除已经完成的上传任务
    });

}

/************************************************
TODO: 显示当前的上传文件列表
************************************************/
void MainWindow::refreshUploadFiles()
{
    //清空当前文件列表所有item项目
    clearItems();

    //获取上传列表实例
    UploadTask *uploadList = UploadTask::getInstance();
    if(uploadList == NULL)
    {
        return;
    }

    //显示上传文件列表
    if(uploadList->isEmpty() == false)
    {
        int n = uploadList->listSize();
        for(int i = 0; i < n; ++i)
        {
            QListWidgetItem *item = uploadList->takeItem(i);
            ui->listWidget->addItem(item);
        }
    }

    //消息框通知信息
    addInform("UPLOAD", QString("当前上传列表有 %1 个待上传文件").arg(uploadList->listSize()));
}

/************************************************
TODO: 将需要下载的文件添加至下载任务列表
************************************************/
void MainWindow::addDownloadFiles()
{
    //获取当前选中的item
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == NULL)
    {
        return;
    }

    //获取下载列表实例
    DownloadTask *p = DownloadTask::getInstance();

    for(int i = 0; i < m_fileList.size(); ++i)
    {
        if(m_fileList.at(i)->item == item)
        {

            QString filePathName = QFileDialog::getSaveFileName(this, "选择保存文件路径", m_fileList.at(i)->filename );
            if(filePathName.isEmpty())
            {
                return;
            }
            //将文件追加到下载列表
            int res = p->appendDownloadList(m_fileList.at(i), filePathName);
            if(res == -1)
            {
                addInform("DOWNLOAD", QString("%1 文件已经在下载队列中").arg(filePathName));
            }
            else if(res == -2) //打开文件失败
            {
                addInform("DOWNLOAD", QString("%1 文件下载失败").arg(filePathName));
            }

            break;
        }
    }

    //下载文件处理
    downloadFilesAction();
}

/************************************************
TODO: 下载文件处理
************************************************/
void MainWindow::downloadFilesAction()
{
    DownloadTask *p = DownloadTask::getInstance();
    if(p == NULL)
    {
        return;
    }

    if(p->isEmpty()) //如果队列为空，说明没有任务
    {
        return;
    }

    if(p->isDownload()) //当前时间没有任务在下载，才能下载，单任务
    {
        return;
    }

    DownloadInfo *tmp = p->takeTask(); //取下载任务

    QUrl url = tmp->url;
    QFile *file = tmp->file;
    QString md5 = tmp->md5;
    QString user = tmp->user;
    QString filename = tmp->filename;

    //发送get请求
    QNetworkReply* reply = m_manager->get(QNetworkRequest(url));

    if(reply == NULL)
    {
        p->dealDownloadTask(); //删除任务
        addInform("DOWNLOAD", QString("文件 %1 下载失败").arg(filename));
        return;
    }

    //获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply, &QNetworkReply::finished, [=]()
    {
        reply->deleteLater();
        addInform("DOWNLOAD", QString("%1 文件下载成功").arg(filename));
        p->dealDownloadTask();      //删除下载任务
        dealFilePv(md5, filename);  //下载文件pv字段处理
    });

    //当有可用数据时，reply 就会发出readyRead()信号，这时就可以将可用的数据保存下来
    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        //如果文件存在，则写入文件
        if (file != NULL)
        {
            file->write(reply->readAll());
        }
    });

}

/************************************************
TODO: 下载文件标志处理
************************************************/
void MainWindow::dealFilePv(QString md5, QString filename)
{
    QNetworkRequest request; //请求对象

    //获取登陆信息实例
    LoginInfoInstance *login = LoginInfoInstance::getInstance(); //获取单例

    //127.0.0.1:80/dealfile?cmd=pv
    QString url = QString("http://%1:%2/dealfile?cmd=pv").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url)); //设置url

    //qt默认的请求头
    //request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray data = setDealFileJson( login->getUser(), login->getToken(), md5, filename); //设置json包

    //发送post请求
    QNetworkReply * reply = m_manager->post(request, data);
    if(reply == NULL)
    {
        return;
    }

    connect(reply, &QNetworkReply::finished, [=]()
    {
        if (reply->error() != QNetworkReply::NoError) //有错误
        {
            addInform("TEST", reply->errorString());
            addInform("TEST", "文件PV处理失败");
            reply->deleteLater(); //释放资源
            return;
        }

        //服务器返回用户的数据
        QByteArray array = reply->readAll();
        reply->deleteLater();

        /*
            下载文件pv字段处理
                成功：{"code":"005"}
                失败：{"code":"018"}
        */
        addInform("TEST", QString("文件PV处理：服务端回复消息 %1").arg(m_cm.getCode(array)));
        if("005" == m_cm.getCode(array) )
        {
            //该文件pv字段+1
            for(int i = 0; i < m_fileList.size(); ++i)
            {
                FileInfo *info = m_fileList.at(i);
                if( info->md5 == md5 && info->filename == filename)
                {
                    int pv = info->pv;
                    info->pv = pv+1;
                    break;
                }
            }
        }
    });

}

/************************************************
TODO: 将user、token、md5和fileName打包成Json格式
************************************************/
QByteArray MainWindow::setDealFileJson(QString user, QString token, QString md5, QString filename)
{
    QMap<QString, QVariant> tmp;
    tmp.insert("user", user);
    tmp.insert("token", token);
    tmp.insert("md5", md5);
    tmp.insert("fileName", filename);

    QJsonDocument jsonDocument = QJsonDocument::fromVariant(tmp);
    if ( jsonDocument.isNull() )
    {
        addInform("TEST", "json is Null of setDealFileJson()");
        return "";
    }

    return jsonDocument.toJson();

}

/************************************************
TODO: 显示下载文件列表
************************************************/
void MainWindow::refreshDownloadFiles()
{
    //清空当前文件列表所有item项目
    clearItems();

    //获取下载列表实例
    DownloadTask *downloadList = DownloadTask::getInstance();
    if(downloadList == NULL)
    {
        return;
    }

    //显示上传文件列表
    if(downloadList->isEmpty() == false)
    {
        int n = downloadList->listSize(); //元素个数
        for(int i = 0; i < n; ++i)
        {
            QListWidgetItem *item = downloadList->takeItem(i);
            ui->listWidget->addItem(item);
        }
    }

    //消息框通知信息
    addInform("DOWNLOAD", QString("当前下载列表有 %1 个待下载文件").arg(downloadList->listSize()));

}

/************************************************
TODO: 清除上传下载任务
************************************************/
void MainWindow::clearAllTask()
{
    //获取上传列表实例
    UploadTask *uploadList = UploadTask::getInstance();
    if(uploadList == NULL)
    {
        return;
    }

    uploadList->clearList();

    //获取下载列表实例
    DownloadTask *p = DownloadTask::getInstance();
    if(p == NULL)
    {
        return;
    }

    p->clearList();
}

/************************************************
TODO: 定时检查处理任务队列中的任务
************************************************/
void MainWindow::checkTaskList()
{
    //定时检查上传队列是否有任务需要上传
    connect(&m_uploadFileTimer, &QTimer::timeout, [=]()
    {
        //上传文件处理，取出上传任务列表的队首任务，上传完后，再取下一个任务
        uploadFilesAction();
    });

    //启动定时器，500毫秒间隔
    // 每个500毫秒，检测上传任务，每一次只能上传一个文件
    m_uploadFileTimer.start(500);

    //定时检查下载队列是否有任务需要下载
    connect(&m_downloadTimer, &QTimer::timeout, [=]()
    {
        //下载文件处理
        downloadFilesAction();
    });

    //启动定时器，500毫秒间隔
    // 每个500毫秒，检测下载任务，每一次只能下载一个文件
    m_downloadTimer.start(500);
}

/************************************************
TODO: 显示主窗口
************************************************/
void MainWindow::showMainWindow()
{
    this->show();

    //初始化状态栏
    // 获取登陆信息实例，从单例中取出 IP/port/usr 的信息
    LoginInfoInstance *login = LoginInfoInstance::getInstance();
    //login->setLoginInfo("tmpUser", "tmpIp", "tmpPort", "token");
    //login->setLoginInfo("test", "81.70.28.177", "80", "1234567890");
    addInform("TEST", QString("%1-%2-%3-%4").arg(login->getIp()).arg(login->getPort()).arg(login->getUser()).arg(login->getToken()));
    QString str = login->getIp() + ":" + login->getPort();
    QLabel *label1 = new QLabel(str, this);
    //label1->setFrameShape(QFrame::WinPanel);
    //label1->setFrameShadow(QFrame::Sunken);
    QLabel *label2 = new QLabel(login->getUser(), this);
    ui->statusbar->addWidget(label1);
    ui->statusbar->addPermanentWidget(label2);

    //刷新用户文件列表
    refreshFiles();
}

/************************************************
TODO: 获取文件所对应的ContentType
************************************************/
QString MainWindow::getContentType(QString str)
{
    //若 m_map 为空，则添加content-type相关的map对象
    if(m_map.isEmpty())
    {
        m_map.insert("html", "text/html");
        m_map.insert("jpg", "image/jpeg");
        m_map.insert("mp3", "audio/mp3");
        m_map.insert("mp4", "video/mpeg4");
        m_map.insert("ppt", "application/x-ppt");
        m_map.insert("txt", "text/plain");
        m_map.insert("pdf", "application/pdf");
    }

    if(m_map.contains(str))
    {
        QMap<QString, QString>::iterator it = m_map.find(str);
        QString ch(it->data());
        return ch;
    }

    return "text/plain";
}

