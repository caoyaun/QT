#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QDebug>
#include "widget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QString title;

    ui->setupUi(this);

    title="QQ";
    this->setWindowTitle(title);

    QIcon icon(":/images/QQ.png");
    this->setWindowIcon(icon);

    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    q = new Widget();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loginButton_clicked()
{
    QString accountStr = ui->accountEdit->text();
    QString passwordStr = ui->passwordEdit->text();

    if (passwordStr == "123")
    {
        qDebug()<<"success";
        this->hide();

        q->get_user_name(accountStr);
        q->show();
    }
    else
    {
        QMessageBox::about(this,"警告",QString("密码错误！"));
        qDebug()<<"fail";
    }

}
