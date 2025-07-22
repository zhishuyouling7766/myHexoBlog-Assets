#ifndef MAINSERVER_H
#define MAINSERVER_H

#include "mytcpserver.h"
#include <QObject>

class MainServer : public QObject  {
    Q_OBJECT
public:
    explicit MainServer(quint16 port = 12345, QObject *parent = nullptr);
    ~MainServer();

    void deviceRegisterSucceed(qintptr socketDescriptor,QString eqCode,QString version);

private slots:
    void onNewConnection(const qintptr handle);

private:
    MyTcpServer *tcpServer;                     // TCP服务器监听器

};

#endif // MAINSERVER_H
