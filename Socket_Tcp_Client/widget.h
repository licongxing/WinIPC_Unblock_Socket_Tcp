#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTime>
#include <windows.h>
#include <winsock2.h>
#include <QDebug>
#define WM_SOCK WM_USER+199

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
private:
    QString getTimeString(){
        QTime time = QTime::currentTime();
        QString str = time.toString("hh:mm:ss");
        return str;
    }

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::Widget *ui;
    WSAData mWsaData;
    SOCKET mSocket;
    sockaddr_in mAddr;
    char mBuf[4096]{0};
};

#endif // WIDGET_H
