#include "SystemConfigManager.h"
#include <QDebug>

SystemConfigManager& SystemConfigManager::getInstance() {
    static SystemConfigManager instance;
    return instance;
}

QString SystemConfigManager::getTcpServerPort() const {
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开配置文件:" << configPath;
        return {};
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "配置文件 XML 解析失败";
        file.close();
        return {};
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomElement tcpPortElem = root.firstChildElement("tcpServerListenPort");
    if (!tcpPortElem.isNull() && tcpPortElem.hasAttribute("number")) {
        return tcpPortElem.attribute("number");
    }

    return {};
}

QString SystemConfigManager::getDeviceIdNameMap(QString eqCode) const {
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开配置文件:" << configPath;
        return {};
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "XML 解析失败";
        file.close();
        return {};
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomElement deviceMapElem = root.firstChildElement("deviceIdForName");
    QDomNodeList deviceNodes = deviceMapElem.elementsByTagName("clientDevice");

    for (int i = 0; i < deviceNodes.count(); ++i) {
        QDomElement deviceElem = deviceNodes.at(i).toElement();
        QString id = deviceElem.attribute("id").trimmed();
        QString name = deviceElem.attribute("name");
        if (id == eqCode.trimmed()) {
            return name;
        }
    }

    return "未知设备";
}
