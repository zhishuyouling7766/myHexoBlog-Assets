#ifndef MAINDATABRIDGE_H
#define MAINDATABRIDGE_H

#include <QObject>
#include "WorkerToMainBridge.h"
#include "maintoworkerbridge.h"

class MainDataBridge : public QObject
{
    Q_OBJECT
public:
    explicit MainDataBridge(QObject* parent = nullptr);
    ~MainDataBridge();

public:
    void broadcastCommandToAll(const Message& message);

    void quitMainToWorkerBridge(qintptr workerId);
    void sendCommandToWorker(qintptr workerId, const Message& message);

public slots:
    MainToWorkerBridge* haveNewMainToWorkerBridge(qintptr workerId);

public:
    WorkerToMainBridge *workerToMainBridge;

    QMutex mutex_allBridge;  // 保护 map 的互斥锁
    QMap<qintptr,MainToWorkerBridge*> allWorkerToMainBridge;

};

#endif // MAINDATABRIDGE_H
