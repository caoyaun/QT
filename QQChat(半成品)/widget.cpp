#include "widget.h"
#include "ui_widget.h"


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    title="QQChat";
    this->setWindowTitle(title);

    QIcon icon(":/images/QQ.png");
    this->setWindowIcon(icon);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::get_user_name(QString nameStr)
{
    QFont font("Microsoft YaHei", 20, 50);
    this->userName = nameStr;
    ui->label->setFont(font);
    ui->label->setText(nameStr);
}

void Widget::on_toolButton_clicked()
{
    editText = ui->lineEdit->text();
    ui->lineEdit->clear();

    qDebug()<<"添加"+editText;

    QIcon *icon = new QIcon(":/images/friend1.png");
    QToolButton *button;
    button = new QToolButton(this);
    button->setIcon(*icon);
    button->setIconSize(QSize(50, 50));
    button->setFixedSize(300,50);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setFont(QFont("Consolas", 12));
    button->setText(editText);

    ui->buttonVLayout->addWidget(button, 0, Qt::AlignTop);          //添加创建的按钮

    connect(button,SIGNAL(clicked()),this,SLOT(onClickedFunc()));
}

void Widget::onClickedFunc()
{
    QToolButton *tmpButton  = (QToolButton *)sender();             //获取发送信号的对象
    QString tmpText = tmpButton->text();

    mdia= new Form();
    mdia->initdlg(tmpText,userName);
    mdia->show();
}
