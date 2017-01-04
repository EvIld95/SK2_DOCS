#-------------------------------------------------
#
# Project created by QtCreator 2016-12-04T15:41:49
#
#-------------------------------------------------

QT       += core gui
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Docs
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    keyenterreceiver.cpp

HEADERS  += mainwindow.h \
    keyenterreceiver.h

FORMS    += mainwindow.ui
