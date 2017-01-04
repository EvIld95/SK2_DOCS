#include "keyenterreceiver.h"
#include "QKeyEvent"
#include "QPlainTextEdit"
#include "mainwindow.h"
#include <iostream>
keyEnterReceiver::keyEnterReceiver(QObject *parent) : QObject(parent)
{

}

bool keyEnterReceiver::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if ( (key->key()==Qt::Key_Backspace)) {
            QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(obj);
            MainWindow* mwindow = dynamic_cast<MainWindow*>(plainText->parentWidget()->parentWidget());
            mwindow->backspaceService();
            //mwindow->backSpace = true;
        } else {
            return QObject::eventFilter(obj, event);
        }
        return false;
    } else {
        return QObject::eventFilter(obj, event);
    }
    return false;
}
