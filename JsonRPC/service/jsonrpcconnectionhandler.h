#ifndef JSONRPCCONNECTIONHANDLER_H
#define JSONRPCCONNECTIONHANDLER_H

#include "message.h"
#include <QObject>
#include <QRunnable>
#include <QTcpSocket>
#include <QJsonObject>
#include <QTimer>

class JsonRpcConnectionHandler : public QObject
{
    Q_OBJECT

public:
    explicit JsonRpcConnectionHandler(qintptr socketDescriptor, QObject* parent = nullptr);
    ~JsonRpcConnectionHandler();

public:
    void onReadyRead(); // 接收到消息~
    void onDisconnected(); //链接完成断开并发送信号
    void onHeartbeatTimeout(); // 心跳超时回调
    void sendRequestHandler(const Message msg);//服务器向客户端发送消息 总处理
private:
    void sendJsonError(const QJsonValue &id, int code, const QString &message); //发送error

//服务器接收通知
private:
    void notificationHandler(const QJsonObject &obj); //通知信息 总处理
    void handleRegister(const QJsonObject &params); //注册
    void handleTimeSyn(const QJsonObject &params); //时间同步

//服务器向客户端发送消息
private:

private:
    void responseHandler(const QJsonObject& obj); //响应信息 总处理
    void responseHandler_error(const QJsonObject& obj); //err 响应信息 总处理

signals:
    void TcpSocketOutSignal();
    void registerSucceed(qintptr socketDescriptor,QString eqCode,QString version); //向主线程发送带参消息 链接已完成标准的注册


public:
    QTcpSocket* socket; //TCP链接

    qintptr socketDescriptor;
    QString clientId;
    QTimer* heartbeatTimer=nullptr;

    bool isRegistered = false;
    QString eqCode;
    QString version;
};

#endif // JSONRPCCONNECTIONHANDLER_H
