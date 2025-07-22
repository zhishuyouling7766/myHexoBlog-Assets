#include "jsonrpcconnectionthread.h"

#include "main.h"
#include <JsonRpcConnectionHandler.h>
#include <QEventLoop>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>

JsonRpcConnectionThread::JsonRpcConnectionThread(qintptr socketDescriptor, QObject* parent)
    : QObject(parent), socketDescriptor(socketDescriptor)
{
    setAutoDelete(true); //当 run() 执行完毕后，Qt 会自动 delete handler（释放你当前线程对象内存）

}
JsonRpcConnectionThread::~JsonRpcConnectionThread() {

}

void JsonRpcConnectionThread::run(){

    mainToWorkerBridge = dataBridge->haveNewMainToWorkerBridge(socketDescriptor); //通道线程开启，立即建立与主线程的数据连接桥

    JsonRpcConnectionHandler *jsonRpcConnectionHandler = new JsonRpcConnectionHandler(socketDescriptor);

    qDebug()<<jsonRpcConnectionHandler->clientId;

    //心跳超时
    connect(jsonRpcConnectionHandler->heartbeatTimer, &QTimer::timeout, jsonRpcConnectionHandler, &JsonRpcConnectionHandler::onHeartbeatTimeout);

    // 3秒，若通道无法完成确切注册，则强制退出，按超时论处
    QTimer::singleShot(3000, jsonRpcConnectionHandler, [jsonRpcConnectionHandler]() {
        if (!jsonRpcConnectionHandler->isRegistered) {
            qDebug()<<"通道规定时间内未完成注册";
            jsonRpcConnectionHandler->onHeartbeatTimeout();  // 链接退出
        }
    });

    //处理主线程发来的消息数据
    connect(mainToWorkerBridge, &MainToWorkerBridge::commandReady, jsonRpcConnectionHandler, [this,jsonRpcConnectionHandler]() {
        while (mainToWorkerBridge->hasCommand()) {
            Message msg = mainToWorkerBridge->takeCommand();
            //判断消息是什么，对消息进行分类传递
            if (msg.type == "command" && msg.target=="allThreads" && msg.action == "closeAllThreads") {
                jsonRpcConnectionHandler->onHeartbeatTimeout();  // 链接退出
            }
            if (msg.type == "request" && msg.target=="sendRequestHandler") {
                jsonRpcConnectionHandler->sendRequestHandler(msg);  // 服务器向客户端发送请求
            }


        }
    }, Qt::QueuedConnection);

    //收 正常内容
    connect(jsonRpcConnectionHandler->socket, &QTcpSocket::readyRead, jsonRpcConnectionHandler,&JsonRpcConnectionHandler::onReadyRead);
    //收 客户端请求断开链接消息
    connect(jsonRpcConnectionHandler->socket, &QTcpSocket::disconnected, jsonRpcConnectionHandler, &JsonRpcConnectionHandler::onDisconnected);



    QEventLoop loop; //创建一个事件循环对象 用于阻塞当前线程
    connect(jsonRpcConnectionHandler, &JsonRpcConnectionHandler::TcpSocketOutSignal, &loop, &QEventLoop::quit);
    loop.exec(); //这会启动一个事件循环 阻塞

    jsonRpcConnectionHandler->deleteLater();

    Message msg("command", "mainDataBrige", "quitMainToWorkerBridge",{{"socketDescriptor", socketDescriptor}});
    dataBridge->workerToMainBridge->pushMessage(msg); //向主线程传递信号：断开数据桥

}

