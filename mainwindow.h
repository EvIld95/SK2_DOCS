#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

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

public:
    void backspaceService();
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
    char* receivedMessage;
};

#endif // MAINWINDOW_H
