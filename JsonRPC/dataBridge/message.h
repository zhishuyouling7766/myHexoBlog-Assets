#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QVariantMap>

struct Message {
    QString type;               // 消息类型 "command"指令、"request"请求、"response"响应
    QString target;             // 目标对象
    QString action;             // 执行动作
    QVariantMap params;         // 参数键值表

    //默认构造（无参数）
    Message() = default;
    //快速初始化一个完整的 Message 对象 （带参数）
    Message(const QString &type, const QString &target, const QString &action, const QVariantMap &params)
        : type(type), target(target), action(action), params(params) {}
};

Q_DECLARE_METATYPE(Message)

#endif // MESSAGE_H
