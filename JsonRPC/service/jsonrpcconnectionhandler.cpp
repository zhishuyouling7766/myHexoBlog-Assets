#include "JsonRpcConnectionHandler.h"

#include "main.h"
#include "qDebug"
#include "qhostaddress.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QEventLoop>
#include <QThread>

JsonRpcConnectionHandler::JsonRpcConnectionHandler(qintptr socketDescriptor, QObject* parent)
    : QObject(parent), socketDescriptor(socketDescriptor)
{

    socket = new QTcpSocket();
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "Socket descriptor 绑定失败：" << socketDescriptor;
        socket->deleteLater();  // 避免内存泄漏
        return;
    }

    clientId = QString("%1:%2")
                .arg(socket->peerAddress().toString()) //IP 地址
                .arg(socket->peerPort());  //端口号

    qDebug() << "新客户端已连接：" << clientId ;


    // 在当前线程中创建 QTimer，保证线程一致
    heartbeatTimer = new QTimer(); // 心跳超时计时器
    heartbeatTimer->setInterval(15000);
    heartbeatTimer->start(); // 启动心跳倒计时

}

JsonRpcConnectionHandler::~JsonRpcConnectionHandler(){
    // 清理定时器
    heartbeatTimer->stop();
    delete heartbeatTimer;

    delete socket; //清理链接对象
}


void JsonRpcConnectionHandler::onReadyRead()
{
   heartbeatTimer->start(); // 重置心跳超时计时器

    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data);

    qDebug() << "接受数据：" << message ;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        // sendJsonError(QJsonValue(), -32700, "JSON 格式解析失败");
        return;
    }
    qDebug() << "接受数据转：" <<doc;

    QJsonObject obj = doc.object();

    if (!obj.contains("jsonrpc") || obj["jsonrpc"].toString() != "2.0") {
        // sendJsonError(QJsonValue(), -32600, "缺失或无效的 jsonrpc 字段");
        return;
    }

    bool hasMethod = obj.contains("method");
    // bool hasId = obj.contains("id");
    bool hasResult = obj.contains("result");
    bool hasError = obj.contains("error");

    if (hasMethod && obj["id"]=="null" && !hasResult && !hasError) {
        notificationHandler(obj); //收到通知
    } else if (hasMethod && obj["id"]!="null" && !hasResult && !hasError) {
        // 收到请求
    } else if (!hasMethod &&  obj["id"]!="null" && hasResult && !hasError) {
        responseHandler(obj); //收到正确响应
    } else if (!hasMethod &&  obj["id"]!="null" && !hasResult && hasError) {
        responseHandler_error(obj); //收到错误响应
    } else {
        // sendJsonError(QJsonValue(), -32600, "无效的 JSON-RPC 消息结构");
    }
}


void JsonRpcConnectionHandler::onDisconnected()
{
    qDebug() << "客户端断开连接" << socket;

    emit TcpSocketOutSignal(); //发送信号，该链接已断开

    socket->disconnect();  //防御性断开所有连接，避免悬空槽

}

void JsonRpcConnectionHandler::onHeartbeatTimeout()
{
    qDebug() << "心跳超时或规定时间内未完成确切注册，尝试断开连接" << socket;
    socket->abort();
}

void JsonRpcConnectionHandler::sendJsonError(const QJsonValue &id, int code, const QString &message) {
    QJsonObject error {
        {"code", code},
        {"message", message}
    };

    QJsonObject response {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", error}
    };

    QJsonDocument doc(response);
    socket->write(doc.toJson(QJsonDocument::Compact));
    socket->flush();
}

void JsonRpcConnectionHandler::notificationHandler(const QJsonObject &obj) {
    QString method = obj["method"].toString();
    QJsonObject params = obj.value("params").toObject();
    QJsonValue id = obj["id"];
    qDebug() << "收到通知方法：" << method << " 参数：" << params;

    if (method == "register") {
        handleRegister(params);
    }else if (method == "timeSyn") {
        handleTimeSyn(params);
    }else if (method == "heartbeat") {
        return; // 心跳包无需进一步处理
    }else {
        // sendJsonError(id, -32601, "方法不存在");
        qDebug() << "收到未知通知：" << method;
    }
}

void JsonRpcConnectionHandler::responseHandler(const QJsonObject& obj) {
    Message msg("response", "MainWindow", "setResponseData", {
                                                             {"responseDataQJsonObject", obj},
                                                            });

    dataBridge->workerToMainBridge->pushMessage(msg);

}
void JsonRpcConnectionHandler::responseHandler_error(const QJsonObject& obj) {
    Message msg("response", "MainWindow", "setErrorResponseData", {
                                                              {"responseErrorQJsonObject", obj},
                                                              {"socketDescriptor", socketDescriptor},
                                                              });

    dataBridge->workerToMainBridge->pushMessage(msg);
}

void JsonRpcConnectionHandler::sendRequestHandler(const Message msg){
    // 从 msg 中提取 JSON 请求对象
    QJsonObject request = msg.params.value("request").toJsonObject();

    // 转为 JSON 文本
    QJsonDocument doc(request);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    // 发送数据到客户端
    socket->write(jsonData);
    socket->flush();

    qDebug() << "发送请求到客户端：" << jsonData;
}

// 实现注册逻辑
void JsonRpcConnectionHandler::handleRegister(const QJsonObject &params) {
    eqCode = params["eqCode"].toString();
    version = params["version"].toString();
    if(isRegistered){
        return; //已经完成过注册 拦截注册信号
    }else{
        isRegistered = true;
    }

    Message msg("command", "MainWindow", "addTcpServer", {
                                                             {"socketDescriptor", socketDescriptor},
                                                             {"clientId", clientId},
                                                             {"eqCode", eqCode},
                                                             {"version", version}
                                                         });

    dataBridge->workerToMainBridge->pushMessage(msg);

}

// 实现时间同步逻辑
void JsonRpcConnectionHandler::handleTimeSyn(const QJsonObject &params) {
    qDebug() << "对时信息：" << params;
}




