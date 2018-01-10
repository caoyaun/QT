#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QtNetwork/QUdpSocket>
#include <QString>
#include <QByteArray>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QElapsedTimer>
#include <QTcpSocket>
#include <QTcpServer>
#include <QScrollArea>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QPushButton>
#include <QToolButton>
#include <QProgressDialog>

#define DEFAULT ""
#define IMAGE   "IMAGESENDFILE"
#define CAMERA  "CAMERASENDFILE"
#define FILE    "FILESENDFILE"

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    QString userName;

    QUdpSocket *serverSocket;
    QUdpSocket *clientSocket;

    QString action;

    QHostAddress addr;
    quint16 port;

    QByteArray msgdata;

    QString friendName;

    quint16 sendTimes;

    QString sendfileName;
    QFile sendfile;

    QString recvfilename;
    QFile recvfile;

    quint16 camerastatus;
    quint16 startphoto;

    QTcpServer *tcpServer;
    QTcpSocket *tcpClient;
    QTcpSocket *receivedSocket;

    QCamera *camera;                            //系统摄像头设备
    QCameraViewfinder *cameraViewFinder;        //摄像头取景器部件
    QCameraImageCapture *cameraImageCapture;    //截图部件
    QPushButton *tmpButton;

    QTimer *timer;

    void initdlg(QString str, QString userName);

private slots:
    void readMessage();

    void sendFilehander(void);

    void recvFilehander(void);

    void acceptConnection();

    void on_msgsendButton_clicked();

    void on_filesendButton_clicked();

    void on_imagesendButton_clicked();

    void on_cameraButton_clicked();

    void cameraImageCaptured(int id, QImage image);

    void cameratimeout();

    void on_kachaButton_clicked();

private:
    Ui::Form *ui;
};

#endif // FORM_H
