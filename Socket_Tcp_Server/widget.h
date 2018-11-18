#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTime>
#include <windows.h>
#include <winsock2.h>
#include <QDebug>

#define WM_SOCK WM_USER+99

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
private slots:
    void on_pushButton_clicked();

private:
    QString getTimeString(){
        QTime time = QTime::currentTime();
        QString str = time.toString("hh:mm:ss");
        return str;
    }
    void dealCommand(SOCKET client);
private:
    Ui::Widget *ui;
    SOCKET mListen;
    SOCKADDR_IN mAddr;
    WSADATA mWsaData;
    QMap<SOCKET,sockaddr_in*> mClientMap;
    char mBuf[4096]{0};
    QString mHelp =  "\n帮助菜单：\n"
                     "     help         :显示帮助菜单\n"
                     "     getsysinfo   :获取server主机信息\n"
                     "     swap         :交换鼠标左右键\n"
                     "     restore      :恢复鼠标左右键\n"
                     "     unknown cmd  :将反射回去\n";
};

#endif // WIDGET_H
