#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string.h>
#include <iostream>
#include "keyenterreceiver.h"
#include <iostream>
#include <QMessageBox>

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
    //tcpSocket->connectToHost("127.0.0.1",1253);

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

    QByteArray readed = tcpSocket->readAll();
    std::cout<<std::endl;

    auto readedString = readed.toStdString();
    QString text;
    int counter = 0;
    for(int k=0;k<readedString.length();k++) {
        text.append(readedString[k]);
        if(readedString[k] == '-') {
            counter++;
            k = (counter*1024) - 1;
        }
    }

    std::cout<<"Received: "<<readedString<<std::endl;
    QStringList commandArray = text.split('-',QString::SkipEmptyParts);

    std::string allText = ui->textField->toPlainText().toStdString();
    for(int i=0;i<commandArray.size();i++) {
        QString oryginalCommand = commandArray[i];
        if(oryginalCommand.isEmpty()) {
            continue;
        }
        QString commandConv = (oryginalCommand + QString("-"));
        std::cout<<"COMMAND: "<<commandConv.toStdString()<<std::endl;

        QStringList stringArray = commandConv.split(':',QString::SkipEmptyParts);
        //std::string allText = ui->textField->toPlainText().toStdString();

        char command = stringArray[1].toStdString()[0];
        if(command == 'r') { // remove character
            int x = stringArray[3].toInt() - 1;
            allText.erase(allText.begin() + x + 1);
        } else if(command == 'f') { //finish connection
            tcpSocket->close();

            if(closeWindow) {
                this->close();
            }

        } else if(command == 'a') { //add new text
            int x = stringArray[3].toInt() - 1;
            if(allText.length() >= x) {
                allText.insert(allText.begin() + x, stringArray[2].toStdString()[0]);
            }
        } else if(command == 'd') { // delete more than one character
            int x = stringArray[3].toInt() - 1;
            int end = stringArray[4].toInt() - 1;
            allText.erase(allText.begin() + x + 1, allText.begin() + end + 1);
        } else if(command == 'k') { //copy all text
            allText = stringArray[2].toStdString(); //kopiowanie
        } else if(command == 'c' && stringArray[2].toStdString()[0] == '-' && (ui->textField->toPlainText().length() > 0)) {
            auto printable = QStringLiteral("b:%1:%2:-").arg('c').arg(ui->textField->toPlainText()); //if new connection send all text to new client
            QByteArray ba = printable.toLatin1();
            const char *c_str2 = ba.data();
            if(tcpSocket->write(c_str2, printable.length()) == -1) {
                std::cout<<"Not send !!!!"<<std::endl;
            }
        } else if(command == 'c' && stringArray.length() > 3) {
            allText = stringArray[2].toStdString(); //receive text
        } else {
            canWrite = true;
            return;
        }
    }
    ui->textField->clear();
    canWrite = false;

    ui->textField->insertPlainText(QString::fromStdString(allText));

}

void MainWindow::on_textField_textChanged()
{
    if(canWrite) {
        char command;
        QString str1;
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
        } else if(this->selectionActive && backSpace == false) {
            command = 'k';
            str1 = ui->textField->toPlainText();
        }
        else {
            if(ui->textField->toPlainText().length() - previousText.length() > 2) {
                command = 'k';
                str1 = ui->textField->toPlainText();
            } else {
                str1 = (ui->textField->toPlainText().at(position-1)).toLatin1();
                command = 'a'; //add
            }
        }

        if(command == 'd') {
            pos = this->startSelection;
        }

        auto printable = QStringLiteral("b:%1:%2:%3:%4:-").arg(command).arg(str1).arg(pos).arg(endSelection);

        QByteArray ba = printable.toLatin1();
        const char *c_str2 = ba.data();
        if(tcpSocket->write(c_str2, printable.length()) == -1) {
            std::cout<<"Not send !!!!"<<std::endl;
        } else {
            std::cout<<"Send: "<<c_str2<<std::endl;
        }
        backSpace = false;
    } else {
        canWrite = true;
    }

    if(count > 0) {
        previousText = ui->textField->toPlainText();
    }
    count++;
}

void MainWindow::on_textField_cursorPositionChanged()
{
    position = ui->textField->textCursor().position();
    xCursor = ui->textField->textCursor().columnNumber();
    yCursor = ui->textField->textCursor().position() /(lineWidth+1);

}

void MainWindow::backspaceService() {
    std::cout<<"BACKSPACE"<<std::endl;
    this->backSpace = true;
}

void MainWindow::disconnected() {
    std::cout<<"DISCONNECTED"<<std::endl;
    ui->textField->setDisabled(true);
    ui->disconnectButton->setDisabled(true);
    ui->connectButton->setDisabled(false);
}

void MainWindow::connected() {
    std::cout<<"CONNECTED"<<std::endl;
    ui->textField->setDisabled(false);
    ui->disconnectButton->setDisabled(false);
    ui->connectButton->setDisabled(true);

    if(tcpSocket->write("b:newUser:-", 12) == -1) {
        std::cout<<"Not send !!!!"<<std::endl;
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


void MainWindow::on_connectButton_clicked()
{
    tcpSocket->connectToHost(ui->serwerTextEdit->text(),ui->portTextEdit->text().toInt());
}

void MainWindow::on_disconnectButton_clicked()
{
    if(tcpSocket->write("b:close:-", 9) == -1) {
        std::cout<<"Not send !!!!"<<std::endl;
    } else {
        std::cout<<"Close command sent"<<std::endl;
    }
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if(closeWindow == false) {
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "SharedDocs",
                                                                    tr("Are you sure?\n"),
                                                                    QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes) {
            this->closeWindow = true;
            if(tcpSocket->write("b:close:-", 9) == -1) {
                std::cout<<"Not send !!!!"<<std::endl;
            } else {
                std::cout<<"Close command sent"<<std::endl;
            }
        }
        event->ignore();
    } else {
        event->accept();
    }
}
