#ifndef JSONRPCCONNECTIONTHREAD_H
#define JSONRPCCONNECTIONTHREAD_H


#include "maintoworkerbridge.h"
#include <QMutex>
#include <QQueue>
#include <QObject>
#include <QRunnable>


class JsonRpcConnectionThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit JsonRpcConnectionThread(qintptr socketDescriptor,QObject* parent = nullptr);
    ~JsonRpcConnectionThread();

    void run() override;

public:
    qintptr socketDescriptor;

    MainToWorkerBridge *mainToWorkerBridge  = nullptr;

};

#endif // JSONRPCCONNECTIONTHREAD_H
