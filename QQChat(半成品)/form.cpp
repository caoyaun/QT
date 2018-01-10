#include "form.h"
#include "ui_form.h"
#include <QString>
#include <QByteArray>
#include <QTime>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{

    ui->setupUi(this);
    ui->msg->setFont(QFont("Consolas", 14));

    camerastatus = 0;
    startphoto = 0;
    tmpButton = ui->cameraButton;

    port=8888;

    /* 聊天用udp */
    serverSocket=new QUdpSocket(this);
    clientSocket=new QUdpSocket(this);

    /* 传输文件用tcp */
    tcpClient = new QTcpSocket(this);
    tcpServer = new QTcpServer(this);

    tcpServer->listen(QHostAddress::Any, port);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

    clientSocket->bind(QHostAddress::AnyIPv4, port);
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));

    camera = new QCamera();
    cameraViewFinder = new QCameraViewfinder();
    cameraImageCapture = new QCameraImageCapture(camera);

    connect(cameraImageCapture, SIGNAL(imageCaptured(int,QImage)), this,SLOT(cameraImageCaptured(int,QImage)));

    cameraImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    camera->setViewfinder(cameraViewFinder);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(cameratimeout()));

}

Form::~Form()
{
    camera->stop();
    delete ui;
}

void Form::initdlg(QString str, QString userName)
{
    friendName = str;

    this->userName = userName;

    addr=QHostAddress(str);

    str = "正在和"+str+"聊天";

    this->setWindowTitle(str);
}

void Form::recvFilehander()
{
    qint64 filesize;
    qint64 readyReadSize = 0;
    float count = 0;
    float sendfilesize;

    qDebug()<<"Enter recvFilehander!";

    if(!recvfile.open(QIODevice::WriteOnly))
    {
        qDebug()<<"recvfile open fail!";
        return;
    }
    recvfile.resize(0);             //清空原有文件内容

    /* 先获取文件大小 */
    receivedSocket->waitForReadyRead(50);
    QDataStream in(receivedSocket);
    in>>filesize;
    qDebug()<<filesize;

    sendfilesize = (float)filesize;
    /* 进度条 */
    QProgressDialog *progressDialog=new QProgressDialog(this);
    QFont font("ZYSong18030",12);
    progressDialog->setFont(font);

    progressDialog->setWindowModality(Qt::WindowModal);
    /*
     * 设置进度对话框采用模态方式进行显示，即显示进度的同时，其他窗口不响应输入信号
     */
    progressDialog->setMinimumDuration(1);
    /*
     * 设置进度对话框需要操作的最短时间，设定为5秒，默认是4秒
     * 如果时间短于minimumduration，则对话框无法出现
     */
    progressDialog->setWindowTitle(tr("正在接收文件，请稍后。。。"));
    progressDialog->setLabelText(tr("recv..."));
    progressDialog->setCancelButtonText(tr("Cancel"));
    /*
     *设置进度对话框的取消按钮的显示文字
     */
    progressDialog->setRange(0,99);

    /* 获取文件内容 */
    while(readyReadSize < filesize)
    {
        receivedSocket->waitForReadyRead(50);
        QByteArray line = receivedSocket->readAll();

        count = count + (float)line.size();

        progressDialog->setValue((int)((count/sendfilesize) * 100));

        recvfile.write(line);
        recvfile.flush();

        readyReadSize += line.size();
    }

    progressDialog->setValue(99);
    progressDialog->wasCanceled();

    recvfile.close();

    if (action == IMAGE)
    {
        ui->msg->append("图片接收完毕!!!");
        ui->msg->setAlignment(Qt::AlignLeft);

        QPixmap image;                          //定义一张图片
        image.load(recvfilename);               //加载
        ui->muliLabel->clear();                 //清空
        ui->muliLabel->setScaledContents(true);
        ui->muliLabel->setPixmap(image);        //加载到Label标签
        ui->muliLabel->show();                  //显示
    }
    else if (action == FILE)
    {
        ui->msg->append("文件接收完毕!!!");
        ui->msg->setAlignment(Qt::AlignLeft);

        if(!recvfile.open(QIODevice::ReadOnly))
        {
            qDebug()<<"recvfile open fail!";
            return;
        }

        QMessageBox::StandardButton rb = QMessageBox::information(NULL, "提示", "是否把文件内容显示在文本框？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(rb == QMessageBox::Yes)
        {
            QString textTmp = recvfile.readAll();
            ui->muliLabel->setFont(QFont("Consolas", 10));

            ui->msg->setTextColor(QColor("black"));
            ui->msg->append(textTmp);

            ui->msg->setTextColor(QColor("green"));
            ui->msg->append("提示,以上为文件内容!!!");

            recvfile.close();
        }
    }
    else if(action == CAMERA)
    {
        ui->msg->append("照片接收完毕!!!");
        ui->msg->setAlignment(Qt::AlignLeft);

        if (!recvfile.open(QIODevice::ReadOnly))
        {
            qDebug()<<"open fail!";
            return;
        }
        QByteArray imageData = recvfile.readAll();
        QPixmap tmpPixmap;

        QDataStream ds(&imageData,QIODevice::ReadOnly);
        ds>>tmpPixmap;

        ui->muliLabel->setPixmap(tmpPixmap);
        sendfile.close();
    }
}

void Form::sendFilehander()
{
    /* 发送文件大小 */
    QByteArray outBlock;
    float count = 0;
    float sendfilesize;

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    out<<qint64(0);
    out.device()->seek(0);
    sendfilesize = (float)sendfile.size();
    out<<(qint64)sendfilesize;

    tcpClient->write(outBlock);                 //将读到的文件大小发送到套接字


    qDebug()<<"Enter sendFilehander!";


    /* 进度条 */
    QProgressDialog *progressDialog=new QProgressDialog(this);
    QFont font("ZYSong18030",12);
    progressDialog->setFont(font);

    progressDialog->setWindowModality(Qt::WindowModal);
    /*
     * 设置进度对话框采用模态方式进行显示，即显示进度的同时，其他窗口不响应输入信号
     */
    progressDialog->setMinimumDuration(1);
    /*
     * 设置进度对话框需要操作的最短时间，设定为5秒，默认是4秒
     * 如果时间短于minimumduration，则对话框无法出现
     */
    progressDialog->setWindowTitle(tr("正在发送文件，请稍后。。。"));
    progressDialog->setLabelText(tr("sending..."));
    progressDialog->setCancelButtonText(tr("Cancel"));
    /*
     *设置进度对话框的取消按钮的显示文字
     */
    progressDialog->setRange(0,99);

    qDebug()<<sendfilesize;

    /* 发送文件 */
    while(!sendfile.atEnd())
    {
        QByteArray line = sendfile.read(8196);
        tcpClient->write(line);                 //将读到的文件发送到套接字
        count = count + (float)line.size();

        progressDialog->setValue((int)((count/sendfilesize) * 100));

        tcpClient->waitForBytesWritten(50);
    }

    progressDialog->setValue(99);
    progressDialog->wasCanceled();

    qDebug()<<"End of send!";

    sendfile.close();

    tcpClient->disconnectFromHost();
}


void Form::readMessage()
{
    QHostAddress addr1;
    quint16 port1;

    QByteArray data;
    qint64 len=clientSocket->pendingDatagramSize();

    data.resize(len);
    clientSocket->readDatagram(data.data(),len,&addr1,&port1);

    QString tmpOrder = data;

    if (tmpOrder.left(13) == IMAGE)
    {
        QMessageBox::StandardButton rb = QMessageBox::information(NULL, "提示", "是否接收图片"+tmpOrder.mid(13)+"?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(rb == QMessageBox::Yes)
        {
            QByteArray imageData="readyOK";
            qDebug()<<"readyOK";

            recvfilename = QFileDialog::getSaveFileName(this,tr("Save Image"),tmpOrder.mid(13),tr("Images (*.png *.bmp *.jpg)")); //选择路径
            recvfile.setFileName(recvfilename);

            clientSocket->writeDatagram(imageData,addr,port);

            ui->msg->append("正在接收图片!!!");
            ui->msg->setAlignment(Qt::AlignLeft);

            action = IMAGE;
        }
    }
    else if(tmpOrder == CAMERA)
    {
        QMessageBox::StandardButton rb = QMessageBox::information(NULL, "提示", "是否接收照片？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(rb == QMessageBox::Yes)
        {
            QByteArray imageData="readyOK";
            qDebug()<<"readyOK";

            recvfilename = QFileDialog::getSaveFileName(this,tr("Save Image"),"",tr("Images (*.png *.bmp *.jpg)")); //选择路径
            recvfile.setFileName(recvfilename);

            clientSocket->writeDatagram(imageData,addr,port);

            ui->msg->append("正在接收照片!!!");
            ui->msg->setAlignment(Qt::AlignLeft);

            action = CAMERA;
        }
    }
    else if (tmpOrder.left(12) == FILE)
    {

        QMessageBox::StandardButton rb = QMessageBox::information(NULL, "提示", "是否接收文件"+tmpOrder.mid(12)+"?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(rb == QMessageBox::Yes)
        {
            QByteArray fileData="readyOK";
            qDebug()<<"readyOK";

            recvfilename = QFileDialog::getSaveFileName(this,tr("Save file"),tmpOrder.mid(12),tr("file (*)")); //选择路径
            recvfile.setFileName(recvfilename);

            clientSocket->writeDatagram(fileData,addr,port);

            ui->msg->append("正在接收文件!!!");
            ui->msg->setAlignment(Qt::AlignLeft);

            action = FILE;

        }
    }
    else if(tmpOrder == "readyOK")
    {
        /* 连接服务器 */
        tcpClient->disconnectFromHost();
        tcpClient->connectToHost(addr, port);
        connect(tcpClient, SIGNAL(connected()), this, SLOT(sendFilehander()));  //当连接成功时，就开始传送文件
    }
    else
    {
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss ddd");
        QColor textClolor("green");
        ui->msg->setTextColor(textClolor);

        ui->msg->setFont(QFont("Consolas", 8));
        ui->msg->append(current_date);
        ui->msg->setAlignment(Qt::AlignLeft);

        QColor textClolor2("black");
        ui->msg->setTextColor(textClolor2);
        ui->msg->setFont(QFont("Consolas", 14));
        ui->msg->append(data);
        ui->msg->setAlignment(Qt::AlignLeft);
    }
}

void Form::acceptConnection()
{
    receivedSocket = tcpServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(recvFilehander()));
}

void Form::on_msgsendButton_clicked()
{
    QString msg;

    msg = ui->msgtextEdit->toPlainText();
    ui->msgtextEdit->clear();

    msgdata.clear();
    msgdata.append(userName+": "+msg);

    /* 显示时间 */
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss ddd");
    QColor textClolor("green");
    ui->msg->setTextColor(textClolor);

    ui->msg->setFont(QFont("Consolas", 3));
    ui->msg->append(current_date);
    ui->msg->setAlignment(Qt::AlignRight);

    QColor textClolor2("black");
    ui->msg->setTextColor(textClolor2);
    ui->msg->setFont(QFont("Consolas", 14));

    /* 显示消息 */
    ui->msg->append(msg+" :"+userName);
    ui->msg->setAlignment(Qt::AlignRight);

    clientSocket->writeDatagram(msgdata,addr,port);
}

void Form::on_filesendButton_clicked()
{
   QByteArray fileData;

   /* 选择一个文件 */
   sendfileName=QFileDialog::getOpenFileName(this,tr("choose file"),"",tr("file (*)"));
   qDebug()<<sendfileName<<endl;
   if(sendfileName.isEmpty())
   {
       return;
   }

   sendfile.setFileName(sendfileName);
   if(!sendfile.open(QIODevice::ReadOnly))
       return;

    /* 发送文件请求 */
   fileData.clear();
   fileData.append(FILE+sendfileName.section('/', -1));
   clientSocket->writeDatagram(fileData,addr,port);
}

void Form::on_imagesendButton_clicked()
{
    QByteArray imageData;

   /* 选择一张图片 */
   sendfileName=QFileDialog::getOpenFileName(this,tr("choose image"),".",tr("Images (*.jpg *.png)"));
   qDebug()<<sendfileName<<endl;
   if(sendfileName.isEmpty())
   {
       return;
   }

   sendfile.setFileName(sendfileName);
   if(!sendfile.open(QIODevice::ReadOnly))
       return;

    /* 发送图片请求 */
   imageData.clear();
   imageData.append(IMAGE+sendfileName.section('/', -1));
   clientSocket->writeDatagram(imageData,addr,port);
}

void Form::on_cameraButton_clicked()
{
    if (camerastatus == 0)
    {
        camera->start();
        camerastatus = 1;
        QToolButton *tmpButton  = (QToolButton *)sender();             //获取发送信号的对象
        tmpButton->setText("关闭摄像头");
        timer->start(100);
    }
    else
    {
        camera->stop();
        camerastatus = 0;
        timer->stop();
        QToolButton *tmpButton  = (QToolButton *)sender();             //获取发送信号的对象
        tmpButton->setText("开启摄像头");
        ui->muliLabel->clear();
    }

}

void Form::cameraImageCaptured(int id, QImage image)
{
    if (startphoto == 1)
    {
        camerastatus = 0;
        camera->stop();
        timer->stop();

        tmpButton->setText("开启摄像头");

        startphoto = 0;
        QPixmap photo = QPixmap::fromImage(image);
        sendfile.setFileName("C:/Users/10284/Desktop/QQChat/images/tmp.png");

        QByteArray imageData;
        QDataStream ds(&imageData,QIODevice::WriteOnly);
        ds<<photo;

        if (!sendfile.open(QIODevice::ReadWrite))
        {
            qDebug()<<"open fail!";
            return;
        }

        sendfile.resize(0);
        sendfile.write(imageData);
        sendfile.close();

        QMessageBox::StandardButton rb = QMessageBox::information(NULL, "提示", "是否发送照片？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(rb == QMessageBox::Yes)
        {
            if (!sendfile.open(QIODevice::ReadWrite))
            {
                qDebug()<<"open fail!";
                return;
            }

            QByteArray imageData;
            /* 发送图片请求 */
           imageData.clear();
           imageData.append(CAMERA);
           clientSocket->writeDatagram(imageData,addr,port);
        }
        ui->muliLabel->clear();
    }
    else
        ui->muliLabel->setPixmap(QPixmap::fromImage(image));
}

void Form::cameratimeout()
{
    cameraImageCapture->capture();
}

void Form::on_kachaButton_clicked()
{
    startphoto = 1;

    cameraImageCapture->capture();
}
