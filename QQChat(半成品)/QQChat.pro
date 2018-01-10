#-------------------------------------------------
#
# Project created by QtCreator 2017-10-10T08:28:00
#
#-------------------------------------------------

QT       += core gui network multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QQChat
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    form.cpp \
    mainwindow.cpp

HEADERS  += widget.h \
    form.h \
    mainwindow.h

FORMS    += widget.ui \
    form.ui \
    mainwindow.ui

RESOURCES += \
    res.qrc
