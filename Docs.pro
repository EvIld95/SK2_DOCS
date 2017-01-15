#-------------------------------------------------
#
# Project created by QtCreator 2016-12-04T15:41:49
#
#-------------------------------------------------

QT       += core gui
QT       += network


win32-g++ {
  QMAKE_CXXFLAGS += -std=c++11
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Docs
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    keyenterreceiver.cpp

HEADERS  += mainwindow.h \
    keyenterreceiver.h

FORMS    += mainwindow.ui
