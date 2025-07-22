#include "initialwidget.h"
#include "ui_initialwidget.h"

#include "main.h"
#include <message.h>
#include <QNetworkInterface>
#include <util/SystemConfigManager.h>

initialWidget::initialWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::initialWidget)
{
    ui->setupUi(this);

    qDebug()<<getLocalIPv4();
    ui->label_2->setText("服务器IP："+getLocalIPv4());
    QString port = SystemConfigManager::getInstance().getTcpServerPort(); //从xml文件获取服务端口号
    ui->label_3->setText("端口号："+port);
}

initialWidget::~initialWidget()
{
    delete ui;
}

void initialWidget::on_pushButton_clicked()
{
    // if (msg.type == "command" && msg.target=="allThreads" && msg.action == "closeAllThreads") {
    //     window->processCommand_addTcpServer(msg.params);
    //     jsonRpcConnectionHandler->onDisconnected(); //命令：通道关闭
    // }

    Message msg("command", "allThreads", "closeAllThreads",{});

    dataBridge->broadcastCommandToAll(msg);
}

QString initialWidget::getLocalIPv4()
{
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &iface : interfaces) {
        // 跳过非运行中、回环、本地虚拟接口
        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning) ||
            iface.flags() & QNetworkInterface::IsLoopBack)
            continue;

        // 过滤掉虚拟网卡（VMware、Loopback 等）
        QString name = iface.humanReadableName().toLower();
        if (name.contains("vmware") || name.contains("virtual") || name.contains("loopback"))
            continue;

        // 遍历地址
        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            QHostAddress addr = entry.ip();
            if (addr.protocol() == QAbstractSocket::IPv4Protocol &&
                addr != QHostAddress::LocalHost) {
                return addr.toString();  // 找到第一个非虚拟、非回环、非断开 IPv4
            }
        }
    }

    return "127.0.0.1";  // fallback
}

