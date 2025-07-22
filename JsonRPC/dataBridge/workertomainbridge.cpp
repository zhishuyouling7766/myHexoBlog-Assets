#include "workertomainbridge.h"

// ================== 子线程 → 主线程 ==================
WorkerToMainBridge::WorkerToMainBridge(QObject* parent)
    : QObject(parent)
{
}

//子线程 消息数据发送
void WorkerToMainBridge::pushMessage(const Message& msg)
{
    QMutexLocker locker(&mutex);
    queue.enqueue(msg);
    emit messageReady();
}

//取出即销毁
Message WorkerToMainBridge::takeMessage()
{
    QMutexLocker locker(&mutex);
    return queue.isEmpty() ? Message() : queue.dequeue();
}

//取出不销毁
Message WorkerToMainBridge::peekMessage()
{
    QMutexLocker locker(&mutex);
    return queue.isEmpty() ? Message() : queue.head();
}

//确认是否存在消息内容
bool WorkerToMainBridge::hasMessage()
{
    QMutexLocker locker(&mutex);
    return !queue.isEmpty();
}
