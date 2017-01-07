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
    connect(tcpSocket,SIGNAL(hostFound()), SLOT(hostFound()));
    tcpSocket->connectToHost("127.0.0.1",1251);

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
    //tcpSocket->read(receivedMessage, 16);
    QByteArray readed = tcpSocket->readAll();
    std::cout<<"Received: "<<readed.toStdString()<<std::endl;
    QStringList stringArray = QString(readed).split(':');
    std::string allText = ui->textField->toPlainText().toStdString();

    char command = stringArray[0].toStdString()[0];
    if(command == 'r') {
        int x = stringArray[2].toInt() - 1;
        allText.erase(allText.begin() + x + 1);
    } else if(command == 'a') {
        int x = stringArray[2].toInt() - 1;
        allText.insert(allText.begin() + x, stringArray[1].toStdString()[0]);
    } else if(command == 'd') {
        int x = stringArray[2].toInt() - 1;
        int end = stringArray[3].toInt() - 1;
        allText.erase(allText.begin() + x + 1, allText.begin() + end + 1);
    } else if(command == 'c' && stringArray[1].toStdString()[0] == 'e') {
        auto printable = QStringLiteral("%1:%2:e").arg('c').arg(ui->textField->toPlainText());
        QByteArray ba = printable.toLatin1();
        const char *c_str2 = ba.data();
        if(tcpSocket->write(c_str2, 1024) == -1) {
            std::cout<<"Not send !!!!"<<std::endl;
        } else {
            std::cout<<"Wysłano"<<std::endl;
        }
    } else if(command == 'c') {
        allText.insert(0,stringArray[1].toStdString());
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

        char str1;
        int pos = position;
        if(backSpace) {
            if(this->selectionActive) {
                command = 'd';
                str1 = '0';
                this->selectionActive = false;
            } else {
                std::cout<<"Typed backspace"<<std::endl;
                str1 = '0';
                command = 'r'; //remove
            }
        }
        else {
           str1 = (ui->textField->toPlainText().at(position-1)).toLatin1();
           command = 'a'; //add
        }

        if(command == 'd') {
            pos = this->startSelection;
        }
        //have to make removal service
        auto printable = QStringLiteral("%1:%2:%3:%4:e").arg(command).arg(str1).arg(pos).arg(endSelection);
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
    char command;
    command = 'c'; //connected
    auto printable = QStringLiteral("%1:e").arg(command);
    QByteArray ba = printable.toLatin1();
    const char *c_str2 = ba.data();
    if(tcpSocket->write(c_str2, 16) == -1) {
        std::cout<<"Not send !!!!"<<std::endl;
    } else {
        std::cout<<"Wysłano"<<std::endl;
    }


}

void MainWindow::error(QAbstractSocket::SocketError error) {
    std::cout<<"ERROR: "<<error<<std::endl;
}

void MainWindow::stateChange(QAbstractSocket::SocketState elem) {
    std::cout<<"STATE: "<<elem<<std::endl;
}

void MainWindow::on_textField_selectionChanged() {
    QTextCursor cursor = ui->textField->textCursor();

    if(!cursor.hasSelection())
        return; // No selection available

    this->startSelection = cursor.selectionStart();
    this->endSelection = cursor.selectionEnd();
    std::cout<<this->startSelection<<" "<<this->endSelection<<std::endl;
    this->selectionActive = true;
}

void MainWindow::hostFound() {
    std::cout<<"FOUND: "<<std::endl;
}

void MainWindow::on_pushButton_2_clicked()
{

}
