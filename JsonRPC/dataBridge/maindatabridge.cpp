#include "maindatabridge.h"

#include <qDebug>
#include <QJsonObject>
#include "main.h"

MainDataBridge::MainDataBridge(QObject* parent)
    : QObject(parent)
{
    workerToMainBridge = new WorkerToMainBridge; //子线程向主线程发送数据将采用多对一模式

    //处理子线程发来的消息数据
    connect(workerToMainBridge, &WorkerToMainBridge::messageReady, this, [=]() {
        while (workerToMainBridge->hasMessage()) {
            Message msg = workerToMainBridge->takeMessage();
            //判断消息是什么，对消息进行分类传递
            if (msg.type == "command" && msg.target=="MainWindow" && msg.action == "addTcpServer") {
                window->mainWidget->processCommand_addTcpServer(msg.params);
                window->stackedWidgetSetCurrentIndex(1);//跳转入可操作页面
            }
            if (msg.type == "command" && msg.target=="mainDataBrige" && msg.action == "quitMainToWorkerBridge"){
                qintptr workerId = msg.params["socketDescriptor"].toLongLong();
                this->quitMainToWorkerBridge(workerId);
            }
            if (msg.type == "response" && msg.target=="MainWindow" && msg.action == "setResponseData"){
                window->mainWidget->setResponseDataForUi(msg.params["responseDataQJsonObject"].toJsonObject());
            }
            if (msg.type == "response" && msg.target=="MainWindow" && msg.action == "setErrorResponseData"){
                window->mainWidget->handleSetResult_error(
                    msg.params["responseErrorQJsonObject"].toJsonObject(),
                    msg.params["socketDescriptor"].toLongLong() );
            }

        }
    }, Qt::QueuedConnection);

}

MainDataBridge::~MainDataBridge(){
    delete workerToMainBridge;

    // 清除所有指针指向的对象
    for (auto it = allWorkerToMainBridge.begin(); it != allWorkerToMainBridge.end(); ++it) {
        delete it.value();  // 释放内存
    }
    allWorkerToMainBridge.clear(); // 清空map本身
}

//向所有子线程发送同一条消息  模拟广播
void MainDataBridge::broadcastCommandToAll(const Message& message) {
    for (auto bridge : allWorkerToMainBridge) {
        if (bridge) {
            bridge->pushCommand(message);
        }
    }
}

//出现新的线程时，创建新的【主线程向子线程】数据连接桥   子线程直接调用
MainToWorkerBridge* MainDataBridge::haveNewMainToWorkerBridge(qintptr workerId){
    QMutexLocker locker(&mutex_allBridge);  // 加锁
    MainToWorkerBridge *mainToWorkerBridge = new MainToWorkerBridge;
    allWorkerToMainBridge.insert(workerId, mainToWorkerBridge);
    qDebug() << allWorkerToMainBridge;
    return mainToWorkerBridge;
}

//退出某【主线程向子线程】数据链接桥   子线程调用
void MainDataBridge::quitMainToWorkerBridge(qintptr workerId){
    QMutexLocker locker(&mutex_allBridge);  // 加锁

    qDebug() <<"准备断开数据桥："<< allWorkerToMainBridge;

    window->mainWidget->processCommand_removeTcpServer(workerId); //删除界面上的相关内容

    auto bridge = allWorkerToMainBridge.value(workerId, nullptr);
    if (bridge) {
        bridge->deleteLater();  // 安全地异步释放 QObject 派生对象
        allWorkerToMainBridge.remove(workerId);
    }

    if(allWorkerToMainBridge.size()==0){
        window->stackedWidgetSetCurrentIndex(0);//跳转入待机页面
    }

    qDebug() <<"已完成断开："<< allWorkerToMainBridge;
}

// 向特定子线程发送消息（点对点）
void MainDataBridge::sendCommandToWorker(qintptr workerId, const Message& message) {
    QMutexLocker locker(&mutex_allBridge); // 保证线程安全

    MainToWorkerBridge* bridge = allWorkerToMainBridge.value(workerId, nullptr);
    if (bridge) {
        bridge->pushCommand(message);  // 推送消息到该子线程
    } else {
        qWarning() << "sendCommandToWorker: 未找到workerId = " << workerId << " 对应的桥接对象";
    }
}
