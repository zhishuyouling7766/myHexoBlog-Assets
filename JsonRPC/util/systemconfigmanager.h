#ifndef SYSTEMCONFIGMANAGER_H
#define SYSTEMCONFIGMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QFile>
#include <QMutex>

class SystemConfigManager
{
public:
    static SystemConfigManager& getInstance();

public:
    QString getTcpServerPort() const;
    QString getDeviceIdNameMap(QString eqCode) const;

private:
    //QT CurveReportXML() = default; 的语法直接指示编译器生成该类的默认构造函数（无参数、不执行任何额外操作）。无论 = default 写在+类内（头文件）还是类外（.cpp），都会生成默认实现。
    SystemConfigManager() = default;  // 私有构造函数
    ~SystemConfigManager() = default; // 私有析构函数
    SystemConfigManager(const SystemConfigManager&) = delete; // 禁止拷贝
    SystemConfigManager& operator=(const SystemConfigManager&) = delete; // 禁止赋值

    QString configPath = ":/resource/mainConfig.xml"; // 配置文件路径

};

#endif // SYSTEMCONFIGMANAGER_H
