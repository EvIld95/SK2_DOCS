#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string.h>
#include <iostream>
#include "keyenterreceiver.h"


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow), tcpSocket(new QTcpSocket(this)) {
    ui->setupUi(this);
    connect(tcpSocket,SIGNAL(readyRead()),SLOT(readData()));
    connect(tcpSocket,SIGNAL(disconnected()),SLOT(disconnected()));
    connect(tcpSocket,SIGNAL(connected()), SLOT(connected()));
    connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(stateChange(QAbstractSocket::SocketState)));
    tcpSocket->connectToHost("127.0.0.1",1250);

    QTextDocument *doc = ui->textField->document();
    QFont font = doc->defaultFont();
    font.setFamily("Courier New");
    doc->setDefaultFont(font);
    //ui->textField->setWordWrapMode(QTextOption::WrapAnywhere);
    keyEnterReceiver* key = new keyEnterReceiver();
    ui->textField->installEventFilter(key);
    canWrite = true;
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::readData() {
    canWrite = false;
    if(receivedMessage == NULL) {
        receivedMessage = new char[16];
    }
    tcpSocket->read(receivedMessage, 16);
    //QByteArray readed = tcpSocket->readAll();
    std::cout<<"Received: "<<receivedMessage<<std::endl;
    QStringList stringArray = QString(receivedMessage).split(':');
    std::string allText = ui->textField->toPlainText().toStdString();
    int x = stringArray[2].toInt() - 1;
    //if x >


    char command = stringArray[0].toStdString()[0];
    if(command == 'r') {
        allText.erase(allText.begin() + x + 1);
    } else if(command == 'a'){
        allText.insert(allText.begin() + x, stringArray[1].toStdString()[0]);
    }
    ui->textField->clear();
    canWrite = false;
    ui->textField->insertPlainText(QString::fromStdString(allText));
    //delete [] receivedMessage;
    //receivedMessage = NULL;
    receivedMessage[0] = '/0';
}

void MainWindow::on_textField_textChanged()
{
    if(canWrite) {
        char command;
        char str1 = (ui->textField->toPlainText().at(position-1)).toLatin1();
        if(backSpace) {
            std::cout<<"Typed backspace"<<std::endl;
            str1 = '0';
            command = 'r'; //remove
        }
        else {
           str1 = (ui->textField->toPlainText().at(position-1)).toLatin1();
           command = 'a'; //add
        }
        //have to make removal service
        auto printable = QStringLiteral("%1:%2:%3:e").arg(command).arg(str1).arg(position);//.arg(yCursor);
        std::cout<<"Text changed: "<<str1<<' '<<xCursor<<yCursor<<std::endl;
        QByteArray ba = printable.toLatin1();
        const char *c_str2 = ba.data();
        if(tcpSocket->write(c_str2, 16) == -1) {
            std::cout<<"Not send !!!!"<<std::endl;
        } else {
            std::cout<<"Wysłano: "<<c_str2<<std::endl;
        }
        backSpace = false;
    } else {
        canWrite = true;
    }

}

void MainWindow::on_textField_cursorPositionChanged()
{
    position = ui->textField->textCursor().position();
    //xCursor = (ui->textField->textCursor().position())%lineWidth;
    xCursor = ui->textField->textCursor().columnNumber();
    yCursor = ui->textField->textCursor().position() /(lineWidth+1);

}

void MainWindow::backspaceService() {
    std::cout<<"BACKSPACE SERVICE"<<std::endl;
    this->backSpace = true;
}

void MainWindow::disconnected() {
    std::cout<<"DISCONNECTED"<<std::endl;
}

void MainWindow::connected() {
    std::cout<<"CONNECTED"<<std::endl;
}

void MainWindow::error(QAbstractSocket::SocketError error) {
    std::cout<<"ERROR: "<<error<<std::endl;
}

void MainWindow::stateChange(QAbstractSocket::SocketState elem) {
    std::cout<<"STATE: "<<elem<<std::endl;
}
