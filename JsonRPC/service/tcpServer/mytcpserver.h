#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject* parent = nullptr);

signals:
    void newClientHandle(qintptr handle);

protected:
    void incomingConnection(qintptr handle) override;
};


#endif // MYTCPSERVER_H
