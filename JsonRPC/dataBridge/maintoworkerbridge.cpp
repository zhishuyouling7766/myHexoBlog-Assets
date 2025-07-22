#include "maintoworkerbridge.h"

MainToWorkerBridge::MainToWorkerBridge(QObject* parent)
    : QObject(parent)
{
}

void MainToWorkerBridge::pushCommand(const Message& msg)
{
    QMutexLocker locker(&mutex);
    commandQueue.enqueue(msg);
    emit commandReady();  // 通知子线程
}

Message MainToWorkerBridge::takeCommand()
{
    QMutexLocker locker(&mutex);
    return commandQueue.isEmpty() ? Message() : commandQueue.dequeue();
}

Message MainToWorkerBridge::peekCommand()
{
    QMutexLocker locker(&mutex);
    return commandQueue.isEmpty() ? Message() : commandQueue.head();
}

bool MainToWorkerBridge::hasCommand()
{
    QMutexLocker locker(&mutex);
    return !commandQueue.isEmpty();
}
