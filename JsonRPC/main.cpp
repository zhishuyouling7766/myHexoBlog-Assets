// #include "precompiled.h"  //导入预编译头，必须放在第一行
#include "main.h"


#include <QApplication>
#include <QThreadPool>

#include <util/SystemConfigManager.h>

MainDataBridge *dataBridge = nullptr;
MainWindow *window = nullptr;
MainServer *server = nullptr;

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    // qputenv("QT_SCALE_FACTOR", "1.25"); // 整个应用缩放为 1.25 倍
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");  // 自动按屏幕分辨率适配

    QApplication a(argc, argv);

    QThreadPool::globalInstance()->setMaxThreadCount(20); // 限制线程池并发连接数
    qRegisterMetaType<Message>();  // 注册消息结构体

    dataBridge = new MainDataBridge; //数据桥
    QString port = SystemConfigManager::getInstance().getTcpServerPort(); //从xml文件获取服务端口号
    server = new MainServer(port.toInt()); //设置服务端口

    window = new MainWindow; //界面
    window->show();


    int ret = a.exec();
    // delete window;  // 手动释放资源，避免内存泄漏
    // delete server;
    // delete dataBridge;
    return ret;
}
