#ifndef MAINTOWORKERBRIDGE_H
#define MAINTOWORKERBRIDGE_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include "message.h"

class MainToWorkerBridge : public QObject
{
    Q_OBJECT
public:
    explicit MainToWorkerBridge(QObject* parent = nullptr);

    void pushCommand(const Message& msg);     // 主线程调用
    Message takeCommand();                    // 子线程调用
    Message peekCommand();
    bool hasCommand();

signals:
    void commandReady();  // 通知子线程有新命令

private:
    QMutex mutex;
    QQueue<Message> commandQueue;
};

#endif // MAINTOWORKERBRIDGE_H
