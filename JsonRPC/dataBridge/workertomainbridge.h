#ifndef WORKERTOMAINBRIDGE_H
#define WORKERTOMAINBRIDGE_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include "message.h" // 自定义的消息结构体

class WorkerToMainBridge : public QObject
{
    Q_OBJECT
public:
    explicit WorkerToMainBridge(QObject* parent = nullptr);

    void pushMessage(const Message& msg);  // 子线程调用
    Message takeMessage();                 // 主线程取消息
    Message peekMessage();
    bool hasMessage();

signals:
    void messageReady();  // 通知主线程有新消息

private:
    QMutex mutex;
    QQueue<Message> queue;
};

#endif // WORKERTOMAINBRIDGE_H
