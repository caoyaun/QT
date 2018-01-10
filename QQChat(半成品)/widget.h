#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QToolBox>
#include <QString>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include "form.h"


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    Form *mdia;
    QString title;
    QString editText;
    QString userName;

    void get_user_name(QString nameStr);

private slots:

    void on_toolButton_clicked();
    void onClickedFunc();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H
