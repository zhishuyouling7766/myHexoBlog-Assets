#include "mytcpserver.h"

MyTcpServer::MyTcpServer(QObject* parent)
    : QTcpServer(parent)
{
}

void MyTcpServer::incomingConnection(qintptr handle)
{
    qDebug() << "信号" << handle;

    emit newClientHandle(handle);
}
