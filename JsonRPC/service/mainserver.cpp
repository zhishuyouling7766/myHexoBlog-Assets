#include "mainserver.h"

#include "jsonrpcconnectionthread.h"
#include "mytcpserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QThreadPool>


MainServer::MainServer(quint16 port, QObject *parent)
    : QObject(parent)
{
    tcpServer = new MyTcpServer(this);

    if (tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "JSON-RPC TCP 服务器启动成功，监听端口：" << port;
    } else {
        qDebug() << "启动 TCP 服务器失败";
    }

    connect(tcpServer, &MyTcpServer::newClientHandle, this, [this](qintptr handle){
        qDebug() << "收到新连接，句柄为：" << handle;
        onNewConnection(handle);
    });

}

MainServer::~MainServer() {
    tcpServer->close();
}

void MainServer::onNewConnection(const qintptr socketDescriptor) {

    auto* thread = new JsonRpcConnectionThread(socketDescriptor);

    QThreadPool::globalInstance()->start(thread);

    // threadmap_temporary.insert(socketDescriptor,thread); //将该线程对象地址记录备份 临时未注册

}

