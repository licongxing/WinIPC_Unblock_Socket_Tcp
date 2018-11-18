#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //winsock 初始化
    WSAStartup(MAKEWORD(2,2),&mWsaData);
    //灰化处理
    ui->lineEdit_2->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
}

Widget::~Widget()
{
    delete ui;
    if(mSocket!=INVALID_SOCKET){
        closesocket(mSocket);
    }
    //winsocket 清理
    WSACleanup();
}

bool Widget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = (MSG*)message;
    switch(msg->message)
    {
    case WM_SOCK:
        //lParam的低位表示通知码
        //这段代码不明白，可以参考MSDN https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-connect
        switch (WSAGETSELECTEVENT(msg->lParam)) {
        case FD_CONNECT:
        {
            //错误码 返回在lParam的高位
            int error = WSAGETSELECTERROR(msg->lParam);
            if(  WSAECONNREFUSED == error || WSAENETUNREACH == error ||WSAETIMEDOUT == error){//连接失败
                ui->lineEdit->setEnabled(true);
                ui->pushButton->setEnabled(true);
                qDebug() << "连接服务器失败";
                ui->listWidget->insertItem(0,QString("%1 连接服务器失败").arg(getTimeString()));
                //放开编辑框和按钮
                ui->lineEdit->setEnabled(true);
                ui->pushButton->setEnabled(true);
            }else{
                qDebug() << "连接服务器成功";
                ui->pushButton->setText("断开连接");
                ui->pushButton->setEnabled(true);

                ui->lineEdit_2->setEnabled(true);
                ui->pushButton_2->setEnabled(true);
                ui->listWidget->insertItem(0,QString("%1 连接服务器成功").arg(getTimeString()));
            }
        }
            break;
        case FD_READ:
            qDebug() << "收到服务端回的消息";
            memset(mBuf,0,4096);
            //收到服务器发送的消息了
            recv(mSocket,mBuf,sizeof(mBuf),0);
            ui->listWidget->insertItem(0,QString("%1 recv:[%2]").arg(getTimeString()).arg(mBuf));
            break;
        case FD_CLOSE:
            //服务端 套接字关闭了
            qDebug() << "服务端关闭了";
            ui->listWidget->insertItem(0,QString("%1 服务端断开连接了").arg(getTimeString()));
            break;
        }
        break;
    }
    //其他交给qt处理
    return QWidget::nativeEvent(eventType, message, result);
}

void Widget::on_pushButton_clicked()
{
    QString but = ui->pushButton->text();
    if(but.compare("断开连接") == 0){
        //断开连接
        qDebug() << "客户端 主动断开连接";
        closesocket(mSocket);//关闭套接字
        ui->listWidget->insertItem(0,QString("%1 客户端主动断开连接").arg(getTimeString()));
        ui->lineEdit->setEnabled(true);
        ui->lineEdit_2->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->pushButton->setText("连接");
        return;
    }

    QString ip = ui->lineEdit->text();
    if(ip.isEmpty())
        return;
    ui->pushButton->setEnabled(false);
    ui->lineEdit->setEnabled(false);

    //创建连接套接字
    mSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //设置非阻塞模式,对 客户端连接到服务器 和 收到消息 通知码 感兴趣
    WSAAsyncSelect(mSocket,(HWND)winId(),WM_SOCK,FD_CONNECT|FD_READ|FD_CLOSE);

    mAddr.sin_addr.S_un.S_addr = inet_addr(ip.toUtf8().data());
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(8888);
    ::connect(mSocket,(sockaddr*)&mAddr,sizeof(mAddr));
}

void Widget::on_pushButton_2_clicked()
{
    QString str = ui->lineEdit_2->text();
    if(str.isEmpty()){
        return;
    }
    char* ch = str.toUtf8().data();//这里的 长度计算一定要这样，防止汉字的长度被计算错误
    send(mSocket,str.toUtf8().data(),strlen(ch)+1,0);
}

void Widget::on_pushButton_3_clicked()
{
    ui->listWidget->clear();
}
