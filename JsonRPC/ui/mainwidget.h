#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QButtonGroup>
#include <QLabel>
#include <QMap>
#include <QSpacerItem>
#include <QStatusBar>
#include <QWidget>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr,QStatusBar *statusbar = nullptr);
    ~MainWidget();

public:
    void processCommand_addTcpServer(QVariantMap dataMap); //有链接进入
    void processCommand_removeTcpServer(qintptr socketDescriptor); //有链接断开
    void clearData();
    void clearID();
    void setResponseDataForUi(QJsonObject request);
    void setResponseDataForUi_uart(QJsonObject request);
    void setResponseDataForUi_can(QJsonObject request);
    void setResponseDataForUi_canfilter(QJsonObject request);
    void setResponseDataForUi_tcp(QJsonObject request);
    void handleSetResult_true(QJsonObject request, const QString& tip = "设置操作已成功完成");
    void handleSetResult_error(QJsonObject request,qintptr socketDescriptor);
    void setErrorToService(int id,int errorCode,QString errorDescription);

private slots:
    void on_pushButton_uart_clicked();

    void on_pushButton_uart_2_clicked();

    void on_pushButton_can_clicked();

    void on_pushButton_can_2_clicked();

    void on_pushButton_canfilter_clicked();

    void on_pushButton_canfilter_2_clicked();

    void on_pushButton_tcp_clicked();

    void on_pushButton_tcp_2_clicked();

    void on_pushButton_getNowDateTime_clicked();

    void on_pushButton_dateTime_clicked();

    void on_pushButton_restart_clicked();

private:
    Ui::MainWidget *ui;
    QStatusBar *statusbar;

    QButtonGroup *buttonGroup;
    QSpacerItem *spacer;

    qintptr channelId = 0;
    int uartid = -1;
    int uartid_2 = -1;
    int canid = -1;
    int canid_2 = -1;
    int canfilterid = -1;
    int canidfilter_2 = -1;
    int tcpid = -1;
    int tcpid_2 = -1;
    int datetimeid = -1;
    int restartid = -1;

    QMap<qintptr,int> allChannelRequestId;

private:
    QDialog* waitingDialog = nullptr;
    QLabel* waitingDialogLabel = nullptr;

};

#endif // MAINWIDGET_H
