#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QMessageBox>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void readData();
    void on_textField_textChanged();
    void on_textField_cursorPositionChanged();
    void disconnected();
    void connected();
    void error(QAbstractSocket::SocketError);
    void stateChange(QAbstractSocket::SocketState);
    void on_textField_selectionChanged();
    void hostFound();

    void on_connectButton_clicked();
    void on_disconnectButton_clicked();

    void on_removeDoc_clicked();
    void on_addDoc_clicked();
    void on_documentList_itemDoubleClicked(QListWidgetItem *item);

public:
    void backspaceService();
    void closeEvent (QCloseEvent *event);

    bool backSpace = false;
    bool selectionActive = false;
    int startSelection = 0;
    int endSelection = 0;
private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    int xCursor;
    int yCursor;

    int position;
    const int lineWidth = 77;
    bool canWrite;
    QString previousText = "";
    int count = 0;
    char* receivedMessage;
    bool closeWindow = false;
    bool clientConnected = false;
};

#endif // MAINWINDOW_H
