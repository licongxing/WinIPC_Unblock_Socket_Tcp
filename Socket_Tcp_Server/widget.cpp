#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //winsock 初始化
    WSAStartup(MAKEWORD(2,2),&mWsaData);

    //创建监听套接字
    this->mListen = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(8888);
    //    mAddr.sin_addr.S_un.S_addr = ADDR_ANY;//服务端可以这样写
    mAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    //绑定本地地址
    bind(mListen,(sockaddr*)&mAddr,sizeof(mAddr));
    //设置监听上限
    listen(mListen,SOMAXCONN);
    //设置mListen为非阻塞模式,应用程序只对FD_ACCEPT感兴趣（有新的socket进行连接，当此事件发生，调用accept函数时为非阻塞直接返回了）
    WSAAsyncSelect(mListen,(HWND)winId(),WM_SOCK,FD_ACCEPT);

}

bool Widget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = (MSG*)message;
    switch(msg->message)
    {
    case WM_SOCK:
        //发生错误统一处理
        if(WSAGETSELECTERROR(msg->lParam)){
            qDebug() << "发生错误";
            break;
        }
        switch (WSAGETSELECTEVENT(msg->lParam)) {
        case FD_ACCEPT:
        {
            qDebug() << "有新的客户端进行连接";

            sockaddr_in  *addr = new sockaddr_in;//传出参数
            int len;//传出参数
            SOCKET client =  accept(mListen,(sockaddr*)addr,&len);
            //通信套接字 对read和close感兴趣
            WSAAsyncSelect(client,(HWND)winId(),WM_SOCK,FD_READ | FD_CLOSE);

            //给客户端发送菜单信息
            char* ch = mHelp.toUtf8().data();
            send(client,ch,strlen(ch)+1,0);

            char* ipStr = inet_ntoa(addr->sin_addr);
            int port = ntohs(addr->sin_port);
            ui->listWidget->insertItem(0,QString("%1 [%2:%3] 连接成功").arg(this->getTimeString()).arg(QString(ipStr)).arg(port));
            //保存到全局对象中
            mClientMap.insert(client,addr);
        }
            break;
        case FD_READ:
        {
            memset(mBuf,0,4096);
            SOCKET client = (SOCKET)msg->wParam;
            sockaddr_in *addr = mClientMap.find(client).value();//msg->wParam保存的是发生网络事件的套接字
            char* ipStr = inet_ntoa(addr->sin_addr);
            int port = ntohs(addr->sin_port);
            recv(client,mBuf,4096,0);

            dealCommand(client);
            ui->listWidget->insertItem(0,QString("%1 [%2:%3]:%4").arg(this->getTimeString()).arg(ipStr).arg(port).arg(mBuf));
        }
            break;
        case FD_CLOSE:
            qDebug() << "客户端断开连接";
            SOCKET client = (SOCKET)msg->wParam;
            sockaddr_in *addr = mClientMap.find(client).value();//msg->wParam保存的是发生网络事件的套接字
            closesocket(client);
            mClientMap.remove(client);

            char* ipStr = inet_ntoa(addr->sin_addr);
            int port = ntohs(addr->sin_port);
            ui->listWidget->insertItem(0,QString("%1 [%2:%3] 断开连接").arg(this->getTimeString()).arg(ipStr).arg(port));
            break;
        }
    }
    //其他交给qt处理
    return QWidget::nativeEvent(eventType, message, result);
}
//处理 客户端发送过来的命令
void Widget::dealCommand(SOCKET client){
    if(QString(mBuf).compare("help") == 0){
        char* ch = mHelp.toUtf8().data();
        send(client,ch,strlen(ch)+1,0);
    }else if(QString(mBuf).compare("swap") == 0){
        SwapMouseButton(true);
        char* ch = "swap命令执行成功";
        send(client,ch,strlen(ch)+1,0);
    }else if(QString(mBuf).compare("restore") == 0 ){
        SwapMouseButton(false);
        char* ch = "restore命令执行成功";
        send(client,ch,strlen(ch)+1,0);
    }else if(QString(mBuf).compare("getsysinfo") == 0){
        char buf[1024]{0};
        DWORD nsize;
        GetComputerNameA(buf,&nsize);
        QString info("\ncomputer name:%1\nuser name:%2\n");
        info  = info.arg(QString(buf));
        memset(buf,0,1024);
        GetUserNameA(buf,&nsize);
        info = info.arg(QString(buf));
        char* ch = info.toUtf8().data();
        send(client,ch,strlen(ch)+1,0);
    }else{
        //未知命令 回射回去
        send(client,mBuf,strlen(mBuf)+1,0);
    }
    return;
}

Widget::~Widget()
{
    //释放与客户端的连接
    for(QMap<SOCKET,sockaddr_in*>::iterator it = mClientMap.begin();it != mClientMap.end();it++){
        delete it.value();
        closesocket(it.key());
    }
    closesocket(mListen);
    delete ui;
    //winsocket 清理
    WSACleanup();
}

void Widget::on_pushButton_clicked()
{
    ui->listWidget->clear();
}
