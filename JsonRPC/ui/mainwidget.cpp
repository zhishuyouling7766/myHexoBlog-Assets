#include "mainwidget.h"
#include "ui_mainwidget.h"

#include "main.h"
#include "message.h"
#include <QAbstractButton>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include "util/uiModule/ipaddress.h"
#include <util/SystemConfigManager.h>


MainWidget::MainWidget(QWidget *parent,QStatusBar *statusbar)
    : QWidget(parent), ui(new Ui::MainWidget), statusbar(statusbar)
{
    ui->setupUi(this);

    // 初始化等待阻塞对话框
    waitingDialog = new QDialog(this, Qt::Dialog | Qt::CustomizeWindowHint);
    waitingDialog->setModal(true);
    waitingDialog->setWindowTitle("请稍候");
    waitingDialog->setFixedSize(350, 150);
    QVBoxLayout* layout = new QVBoxLayout(waitingDialog);
    waitingDialogLabel = new QLabel(waitingDialog);
    waitingDialogLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(waitingDialogLabel);
    waitingDialog->setLayout(layout);

    //为输入框们添加输入限制
    ui->lineEdit_uart_baudrate->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_uart_dataBits->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_uart_stopBits->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_can_baudrate->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_canfilter_count->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_canfilter_ids1->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_canfilter_ids2->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->lineEdit_tcp_port->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));

    ui->dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");

    buttonGroup = new QButtonGroup;
    buttonGroup->setExclusive(true); // 设置按钮组为互斥
    spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

    ui->scrollArea->setWidgetResizable(true); //scrollArea 会在内容溢出时显示滚动条
    // 设置垂直滚动条为“需要时显示”
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 设置水平滚动条为“始终隐藏”
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->scrollAreaWidgetContents->layout()->addItem(spacer);

    clearData();

}

MainWidget::~MainWidget()
{
    delete ui;
    for (QAbstractButton* button : buttonGroup->buttons()) {
        delete button;
    }
    delete buttonGroup;
    delete spacer;
}

void MainWidget::processCommand_addTcpServer(QVariantMap dataMap){

    // msg.params["socketDescriptor"] = socketDescriptor;
    // msg.params["clientId"] = clientId;
    // msg.params["eqCode"] = eqCode;
    // msg.params["version"] = version;

    QString eqCode = dataMap["eqCode"].toString();
    QString version = dataMap["version"].toString();
    QString clientId = dataMap["clientId"].toString();
    qintptr socketDescriptor = dataMap["socketDescriptor"].toLongLong();

    allChannelRequestId[socketDescriptor] = 0; //为通道设置请求id序列

    // 插入新按钮到倒数第二个位置（即弹簧前）
    // 转换为 QVBoxLayout
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->scrollAreaWidgetContents->layout());
    int insertIndex = layout->count() - 1; // 弹簧前的位置
    QString buttonName = SystemConfigManager::getInstance().getDeviceIdNameMap(eqCode);//从xml文件获取通道名称
    QPushButton* button = new QPushButton(buttonName);
    button->setCheckable(true); // 设置按钮可被“选中”
    button->setFixedSize(140, 50);
    buttonGroup->addButton(button,socketDescriptor); //将按钮添加入按钮组
    if (layout) {
        layout->insertWidget(insertIndex, button);
    }

    connect(button, &QPushButton::clicked, this, [this,eqCode,version,clientId,socketDescriptor]() {
        clearData();
        QString textStatusbar = "设备号:"+eqCode+"    "+"版本号:"+version+"    "+"IP:"+clientId;
        statusbar->showMessage(textStatusbar);
        channelId = socketDescriptor;
    });

}

//删除某通道对象
void MainWidget::processCommand_removeTcpServer(qintptr socketDescriptor) {
    // 1. 获取按钮指针
    QAbstractButton* button = buttonGroup->button(socketDescriptor);
    if (!button)
        return;

    // 2. 判断当前是否选中的是该按钮
    if (buttonGroup->checkedButton() == button) {
        clearData();
    }

    // 3. 从按钮组移除（不会删除按钮本体）
    buttonGroup->removeButton(button);

    // 4. 从布局中移除按钮（不会删除按钮本体）
    if (button->parentWidget() && button->parentWidget()->layout()) {
        button->parentWidget()->layout()->removeWidget(button);
    }

    // 5. 断开所有信号槽（可选但更稳）
    button->disconnect();

    // 6. 删除按钮（彻底释放资源）
    delete button;

    allChannelRequestId.remove(socketDescriptor); //删除自增id
    clearData(); //重置显示区
    //关闭阻塞用dialog
    if (waitingDialog) {
        waitingDialog->hide();
    }
}

void MainWidget::clearID() {
    uartid = -1;
    uartid_2 = -1;
    canid = -1;
    canid_2 = -1;
    canfilterid = -1;
    canidfilter_2 = -1;
    tcpid = -1;
    tcpid_2 = -1;
    datetimeid = -1;
    restartid = -1;
}

//重置相关数据显示
void MainWidget::clearData() {
    statusbar->showMessage("");

    // 清空内部变量
    channelId = 0;
    uartid = -1;
    uartid_2 = -1;
    canid = -1;
    canid_2 = -1;
    canfilterid = -1;
    canidfilter_2 = -1;
    tcpid = -1;
    tcpid_2 = -1;
    datetimeid = -1;
    restartid = -1;

    // 重置 UART 参数控件
    ui->comboBox_uart_name->setCurrentIndex(0);
    ui->comboBox_uart_parity->setCurrentIndex(0);
    ui->comboBox_uart_protcal->setCurrentIndex(0);
    ui->comboBox_uart_mode->setCurrentIndex(0);
    // 清空 UART 输入框数值
    ui->lineEdit_uart_baudrate->clear();
    ui->lineEdit_uart_dataBits->clear();
    ui->lineEdit_uart_stopBits->clear();

    // 重置 CAN 参数控件
    ui->comboBox_can_name->setCurrentIndex(0);
    ui->comboBox_can_protcal->setCurrentIndex(0);
    // 清空 CAN 输入框数值
    ui->lineEdit_can_baudrate->clear();

    // 重置 CANFILTER 参数控件
    ui->comboBox_canfilter_name->setCurrentIndex(0);
    ui->comboBox_canfilter_mode->setCurrentIndex(0);
    // 清空 CANFILTER 输入框数值
    ui->lineEdit_canfilter_count->clear();
    ui->lineEdit_canfilter_ids1->clear();
    ui->lineEdit_canfilter_ids2->clear();

    // 重置 TCP 参数控件
    ui->comboBox_tcp_name->setCurrentIndex(0);
    ui->comboBox_tcp_protcal->setCurrentIndex(0);
    ui->comboBox_tcp_isRun->setCurrentIndex(0);
    // 清空 TCP 输入框数值
    ui->ipaddress_tcp_ip->clear();
    ui->lineEdit_tcp_port->clear();

    // 重置 设置 时间 key
    QDateTime currentDateTime = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(currentDateTime);
    ui->lineEdit_restart_key->setText("abcdefg");

}

void MainWidget::setResponseDataForUi(QJsonObject request){
    int id = request["id"].toInt();

    //关闭阻塞用dialog
    if (waitingDialog) {
        waitingDialog->hide();
    }

    if (id == uartid) {
        // 处理 UART 读取请求响应
        setResponseDataForUi_uart(request);
    } else if (id == canid) {
        // 处理 CAN 读取请求响应
        setResponseDataForUi_can(request);
    } else if (id == canfilterid) {
        // 处理 CAN Filter 读取请求响应
        setResponseDataForUi_canfilter(request);
    } else if (id == tcpid) {
        // 处理 TCP 读取请求响应
        setResponseDataForUi_tcp(request);
    } else if (id == uartid_2) {
        // 处理 UART 设置响应 （成功）
        handleSetResult_true(request, "串口设置已完成");
    } else if (id == canid_2) {
        // 处理 CAN 设置响应 （成功与否）
        handleSetResult_true(request, "CAN 设置已完成");
    } else if (id == canidfilter_2) {
        // 处理 CAN Filter 设置响应 （成功与否）
        handleSetResult_true(request, "CAN 滤波器设置已完成");
    } else if (id == tcpid_2) {
        // 处理 TCP 设置响应 （成功与否）
        handleSetResult_true(request, "TCP 设置已完成");
    } else if (id == datetimeid) {
        // 处理 对时 设置响应 （成功与否）
        handleSetResult_true(request, "设备时间同步已完成");
    } else if (id == restartid) {
        // 处理 重启 设置响应 （成功与否）
        handleSetResult_true(request, "设备已重启完成");
    }else {
        QString msg = QString("未识别的响应 ID：%1\n该响应无法被处理").arg(id);
        QMessageBox::warning(this, "错误响应", msg);
        clearID(); //清除失败id处理序列
    }
}

void MainWidget::on_pushButton_uart_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    int currentRequestId = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QJsonObject params;
    params["name"] = ui->comboBox_uart_name->currentText();

    QJsonObject request;
    request["jsonrpc"] = "2.0";            // 协议版本
    request["method"] = "getUartPara";     // 方法名
    request["id"] = currentRequestId;      // 请求唯一标识
    request["params"] = params;            // 参数对象

    Message msg("request", "sendRequestHandler", "readUartConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    uartid = currentRequestId;

    waitingDialogLabel->setText("正在获取串口配置，请稍候...");
    waitingDialog->show();

}

void MainWidget::setResponseDataForUi_uart(QJsonObject request){
    // 检查 result 对象
    if (!request.contains("result") || !request["result"].isObject()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}

    int id = request["id"].toInt();
    QJsonObject result = request.value("result").toObject();

    // 检查 status 和 config
    if (!result.contains("status") || !result["status"].isString()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}
    if (!result.contains("config") || !result["config"].isObject()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}

    QString status = result["status"].toString();
    if(status!="success"){
        // 弹窗提示 响应是错误失败的 不需要后续处理
        QMessageBox::information(this, "响应错误", QString("%1（ID: %2）").arg("串口配置读取失败").arg(id));
        return;
    }


    QJsonObject config = result.value("config").toObject();

    // 检查 config 内部字段完整性
    QStringList requiredKeys = {"baudrate", "dataBits", "stopBits", "parity", "protcal", "mode"};
    for (const QString& key : requiredKeys) {
        if (!config.contains(key) || !config[key].isDouble()) {
            QMessageBox::warning(this, "错误响应", "响应数据结构不完整");
            return;
        }
    }

    // 提取各字段
    int baudrate = config["baudrate"].toInt();
    int dataBits = config["dataBits"].toInt();
    int stopBits = config["stopBits"].toInt();
    int parity = config["parity"].toInt();
    int protcal = config["protcal"].toInt();
    int mode = config["mode"].toInt();

    // 协议类型解析
    int index_protcal;
    switch (protcal) {
    case 2: index_protcal = 0; break; // 80G雷达协议
    case 5: index_protcal = 1; break; // 透明转发协议
    default: QMessageBox::warning(this, "错误响应", "串口协议类型数据错误");  return;
    }

    // 设置各项 UI 控件
    ui->lineEdit_uart_baudrate->setText(QString::number(baudrate));
    ui->lineEdit_uart_dataBits->setText(QString::number(dataBits));
    ui->lineEdit_uart_stopBits->setText(QString::number(stopBits));

    if (parity >= 0 && parity <= 2){
        ui->comboBox_uart_parity->setCurrentIndex(parity);
    }else{
        QMessageBox::warning(this, "错误响应", "串口校验数据错误");
        return;
    }

    ui->comboBox_uart_protcal->setCurrentIndex(index_protcal);

    if (mode >= 0 && mode <= 1){
        ui->comboBox_uart_mode->setCurrentIndex(mode);
    }else{
        QMessageBox::warning(this, "错误响应", "串口模式数据错误");
        return;
    }

    QMessageBox::information(this, "读取成功", QString("串口配置读取成功（ID: %1）").arg(id));
    clearID(); //重置界面逻辑id 逻辑已完成闭环
}

void MainWidget::on_pushButton_uart_2_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    QJsonObject request;
    QJsonObject params;
    QJsonObject config;

    // 提取 UI 字符串并转换为对应类型
    QString jsonrpc = "2.0";
    QString method = "setUartPara";
    int id = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QString name = ui->comboBox_uart_name->currentText();

    int baudrate = ui->lineEdit_uart_baudrate->text().toInt();
    int dataBits = ui->lineEdit_uart_dataBits->text().toInt();
    int stopBits = ui->lineEdit_uart_stopBits->text().toInt();
    int parity = ui->comboBox_uart_parity->currentIndex();
    int protcal = ui->comboBox_uart_protcal->currentIndex();
    switch (protcal) {
        case 0: protcal=2; break;
        case 1: protcal=5; break;
    default:
        break;
    }
    int mode = ui->comboBox_uart_mode->currentIndex();

    // 构造 config 对象
    config["baudrate"] = baudrate;
    config["dataBits"] = dataBits;
    config["stopBits"] = stopBits;
    config["parity"] = parity;
    config["protcal"] = protcal;
    config["mode"] = mode;

    // 构造 params 对象
    params["name"] = name;
    params["config"] = config;

    // 构造 request 对象
    request["jsonrpc"] = jsonrpc;
    request["method"] = method;
    request["id"] = id;
    request["params"] = params;

    Message msg("request", "sendRequestHandler", "writeUartConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    uartid_2 = id;

    waitingDialogLabel->setText("正在设置串口配置，请稍候...");
    waitingDialog->show();
}

void MainWidget::on_pushButton_can_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    int currentRequestId = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QJsonObject params;
    params["name"] = ui->comboBox_can_name->currentText();

    QJsonObject request;
    request["jsonrpc"] = "2.0";            // 协议版本
    request["method"] = "getCanPara";     // 方法名
    request["id"] = currentRequestId;      // 请求唯一标识
    request["params"] = params;            // 参数对象

    Message msg("request", "sendRequestHandler", "readCanConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    canid = currentRequestId;

    waitingDialogLabel->setText("正在获取can配置，请稍候...");
    waitingDialog->show();
}

void MainWidget::setResponseDataForUi_can(QJsonObject request){
    // 检查 result 对象
    if (!request.contains("result") || !request["result"].isObject()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}

    int id = request["id"].toInt();
    QJsonObject result = request.value("result").toObject();

    // 检查 status 和 config
    if (!result.contains("status") || !result["status"].isString()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}
    if (!result.contains("config") || !result["config"].isObject()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}

    QString status = result["status"].toString();
    if(status!="success"){
        // 弹窗提示 响应是错误失败的 不需要后续处理
        QMessageBox::information(this, "响应错误", QString("%1（ID: %2）").arg("can配置读取失败").arg(id));
        return;
    }


    QJsonObject config = result.value("config").toObject();

    // 检查 config 内部字段完整性
    QStringList requiredKeys = {"baudrate", "protcal"};
    for (const QString& key : requiredKeys) {
        if (!config.contains(key) || !config[key].isDouble()) {
            QMessageBox::warning(this, "错误响应", "响应数据结构不完整");
            return;
        }
    }

    // 提取各字段
    int baudrate = config["baudrate"].toInt();
    int protcal = config["protcal"].toInt();

    // 协议类型解析
    int index_protcal;
    switch (protcal) {
    case 4: index_protcal = 0; break; // 透明转发协议
    default: QMessageBox::warning(this, "错误响应", "can协议类型数据错误");  return;
    }


    ui->lineEdit_can_baudrate->setText(QString::number(baudrate));
    ui->comboBox_can_protcal->setCurrentIndex(index_protcal);

    QMessageBox::information(this, "读取成功", QString("can配置读取成功（ID: %1）").arg(id));
    clearID(); //重置界面逻辑id 逻辑已完成闭环
}

void MainWidget::on_pushButton_can_2_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    QJsonObject request;
    QJsonObject params;
    QJsonObject config;

    // 提取 UI 字符串并转换为对应类型
    QString jsonrpc = "2.0";
    QString method = "setCanPara";
    int id = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QString name = ui->comboBox_can_name->currentText();

    int baudrate = ui->lineEdit_can_baudrate->text().toInt();
    int protcal = ui->comboBox_uart_protcal->currentIndex();
    switch (protcal) {
    case 0: protcal=4; break;
    default:
        break;
    }

    // 构造 config 对象
    config["baudrate"] = baudrate;
    config["protcal"] = protcal;

    // 构造 params 对象
    params["name"] = name;
    params["config"] = config;

    // 构造 request 对象
    request["jsonrpc"] = jsonrpc;
    request["method"] = method;
    request["id"] = id;
    request["params"] = params;

    Message msg("request", "sendRequestHandler", "writeCanConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    canid_2 = id;

    waitingDialogLabel->setText("正在设置can配置，请稍候...");
    waitingDialog->show();
}

void MainWidget::on_pushButton_canfilter_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    int currentRequestId = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QJsonObject params;
    params["name"] = ui->comboBox_canfilter_name->currentText();

    QJsonObject request;
    request["jsonrpc"] = "2.0";            // 协议版本
    request["method"] = "getCanLvBoPara";     // 方法名
    request["id"] = currentRequestId;      // 请求唯一标识
    request["params"] = params;            // 参数对象

    Message msg("request", "sendRequestHandler", "readCanfilterConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    canfilterid = currentRequestId;

    waitingDialogLabel->setText("正在获取can滤波配置，请稍候...");
    waitingDialog->show();
}

void MainWidget::setResponseDataForUi_canfilter(QJsonObject request){
    // 检查 result 对象
    if (!request.contains("result") || !request["result"].isObject()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}

    int id = request["id"].toInt();
    QJsonObject result = request.value("result").toObject();

    // 检查 status 和 config
    if (!result.contains("status") || !result["status"].isString()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}
    if (!result.contains("config") || !result["config"].isObject()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}

    QString status = result["status"].toString();
    if(status!="success"){
        // 弹窗提示 响应是错误失败的 不需要后续处理
        QMessageBox::information(this, "响应错误", QString("%1（ID: %2）").arg("can滤波配置读取失败").arg(id));
        return;
    }


    QJsonObject config = result.value("config").toObject();

    // 检查 config 内部字段完整性
    QStringList requiredKeys = {"count", "mode", "ids"};
    if (!config.contains("count") || !config["count"].isDouble() ||
        !config.contains("mode")  || !config["mode"].isDouble()  ||
        !config.contains("ids")   || !config["ids"].isArray()) {
        QMessageBox::warning(this, "错误响应", "响应数据结构不完整");
        return;
    }

    // 提取各字段
    int count = config["count"].toInt();
    int mode = config["mode"].toInt();
    QJsonArray idsArray = config["ids"].toArray();
    QStringList idsList;
    for (const QJsonValue& val : idsArray) {
        quint32 id = static_cast<quint32>(val.toDouble());
        idsList << QString::number(id);  // 直接显示十进制
    }
    QString part1 = idsList.value(0);
    QString part2 = idsList.value(1);
    ui->lineEdit_canfilter_count->setText(QString::number(count));
    if (mode >= 0 && mode <= 1){
        ui->comboBox_canfilter_mode->setCurrentIndex(mode);
    }else{
        QMessageBox::warning(this, "错误响应", "can滤波模式数据错误");
        return;
    }
    ui->lineEdit_canfilter_ids1->setText(part1);
    ui->lineEdit_canfilter_ids2->setText(part2);

    QMessageBox::information(this, "读取成功", QString("can滤波配置读取成功（ID: %1）").arg(id));
    clearID(); //重置界面逻辑id 逻辑已完成闭环
}

void MainWidget::on_pushButton_canfilter_2_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    QJsonObject request;
    QJsonObject params;
    QJsonObject config;

    // 提取 UI 字符串并转换为对应类型
    QString jsonrpc = "2.0";
    QString method = "setCanLvBoPara ";
    int id = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QString name = ui->comboBox_canfilter_name->currentText();

    int count = ui->lineEdit_canfilter_count->text().toInt();
    int mode = ui->comboBox_canfilter_mode->currentIndex();
    QJsonArray idsArray;
    // 从第一个输入框获取 ID
    bool ok1;
    quint32 id1 = ui->lineEdit_canfilter_ids1->text().toUInt(&ok1, 0); // 自动识别 0x 前缀为十六进制
    if (ok1)
        idsArray.append(static_cast<double>(id1));
    // 从第二个输入框获取 ID
    bool ok2;
    quint32 id2 = ui->lineEdit_canfilter_ids2->text().toUInt(&ok2, 0);
    if (ok2)
        idsArray.append(static_cast<double>(id2));

    // 构造 config 对象
    config["count"] = count;
    config["mode"] = mode;
    config["idsArray"] = idsArray;

    // 构造 params 对象
    params["name"] = name;
    params["config"] = config;

    // 构造 request 对象
    request["jsonrpc"] = jsonrpc;
    request["method"] = method;
    request["id"] = id;
    request["params"] = params;

    Message msg("request", "sendRequestHandler", "writeCanfilterConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    canidfilter_2 = id;

    waitingDialogLabel->setText("正在设置can滤波配置，请稍候...");
    waitingDialog->show();
}


void MainWidget::on_pushButton_tcp_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    int currentRequestId = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QJsonObject params;
    params["name"] = ui->comboBox_tcp_name->currentText();

    QJsonObject request;
    request["jsonrpc"] = "2.0";            // 协议版本
    request["method"] = "getTcpPara";     // 方法名
    request["id"] = currentRequestId;      // 请求唯一标识
    request["params"] = params;            // 参数对象

    Message msg("request", "sendRequestHandler", "readTcpConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    tcpid = currentRequestId;

    waitingDialogLabel->setText("正在获取TCP配置，请稍候...");
    waitingDialog->show();
}

void MainWidget::setResponseDataForUi_tcp(QJsonObject request){
    // 检查 result 对象
    if (!request.contains("result") || !request["result"].isObject()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}

    int id = request["id"].toInt();
    QJsonObject result = request.value("result").toObject();

    // 检查 status 和 config
    if (!result.contains("status") || !result["status"].isString()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}
    if (!result.contains("config") || !result["config"].isObject()){ QMessageBox::warning(this, "错误响应", "响应数据结构不完整"); return;}

    QString status = result["status"].toString();
    if(status!="success"){
        // 弹窗提示 响应是错误失败的 不需要后续处理
        QMessageBox::information(this, "响应错误", QString("%1（ID: %2）").arg("串口配置读取失败").arg(id));
        return;
    }


    QJsonObject config = result.value("config").toObject();

    // 检查 config 内部字段完整性
    if (!config.contains("ip") || !config["ip"].isString()
        || !config.contains("port") || !config["port"].isDouble()
        || !config.contains("protcal") || !config["protcal"].isDouble()
        || !config.contains("isRun") || !config["isRun"].isDouble())
    {
        QMessageBox::warning(this, "错误响应", "响应数据结构不完整");
        return;
    }

    // 提取各字段
    QString ip = config["ip"].toString();
    int port = config["port"].toInt();
    int protcal = config["protcal"].toInt();
    int isRun = config["isRun"].toInt();

    int index_protcal = 0;
    switch (protcal) {
    case 1: index_protcal = 0; break; //透明上送协议
    case 6: index_protcal = 1; break; //json rpc 2.0协议
    default: QMessageBox::warning(this, "错误响应", "TCP协议类型数据错误");  break;
    }


    ui->ipaddress_tcp_ip->setIP(ip);
    ui->lineEdit_tcp_port->setText(QString::number(port));
    ui->comboBox_tcp_protcal->setCurrentIndex(index_protcal);
    if (isRun >= 0 && isRun <= 1){
        ui->comboBox_tcp_isRun->setCurrentIndex(isRun);
    }else{
        QMessageBox::warning(this, "错误响应", "TCP运行状态数据错误");
        return;
    }

    QMessageBox::information(this, "读取成功", QString("TCP配置读取成功（ID: %1）").arg(id));
    clearID(); //重置界面逻辑id 逻辑已完成闭环

}

void MainWidget::on_pushButton_tcp_2_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    QJsonObject request;
    QJsonObject params;
    QJsonObject config;

    // 提取 UI 字符串并转换为对应类型
    QString jsonrpc = "2.0";
    QString method = "setTcpPara";
    int id = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QString name = ui->comboBox_tcp_name->currentText();

    QString ip = ui->ipaddress_tcp_ip->getIP();
    int port = ui->lineEdit_tcp_port->text().toInt();
    int protcal = ui->comboBox_tcp_protcal->currentIndex();
    switch (protcal) {
    case 0: protcal=1; break;
    case 1: protcal=6; break;
    default:
        break;
    }
    int isRun = ui->comboBox_tcp_isRun->currentIndex();

    // 构造 config 对象
    config["ip"] = ip;
    config["port"] = port;
    config["protcal"] = protcal;
    config["isRun"] = isRun;

    // 构造 params 对象
    params["name"] = name;
    params["config"] = config;

    // 构造 request 对象
    request["jsonrpc"] = jsonrpc;
    request["method"] = method;
    request["id"] = id;
    request["params"] = params;

    Message msg("request", "sendRequestHandler", "writeTcpConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    tcpid_2 = id;

    waitingDialogLabel->setText("正在设置TCP配置，请稍候...");
    waitingDialog->show();
}


void MainWidget::on_pushButton_getNowDateTime_clicked()
{
    // 获取当前系统日期时间
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 设置到 QDateTimeEdit 控件中
    ui->dateTimeEdit->setDateTime(currentDateTime);
}


void MainWidget::on_pushButton_dateTime_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    int currentRequestId = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QJsonObject params;
    params["time"] = ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");

    QJsonObject request;
    request["jsonrpc"] = "2.0";            // 协议版本
    request["method"] = "timeSyn";      // 方法名
    request["id"] = currentRequestId;      // 请求唯一标识
    request["params"] = params;            // 参数对象

    Message msg("request", "sendRequestHandler", "writeDatetimeConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    datetimeid = currentRequestId;

    waitingDialogLabel->setText("正在设置客户端日期时间配置，请稍候...");
    waitingDialog->show();
}


void MainWidget::on_pushButton_restart_clicked()
{
    // 判断当前是否已选中设备（channelId 有效）
    if (channelId == 0 || !allChannelRequestId.contains(channelId)) {
        QMessageBox::warning(this, "错误", "未选择设备，请先点击左侧设备选择按钮！");
        return;
    }

    int currentRequestId = allChannelRequestId[channelId]+=1; // 获取ID 并自增1

    QJsonObject params;
    params["key"] = ui->lineEdit_restart_key->text();

    QJsonObject request;
    request["jsonrpc"] = "2.0";            // 协议版本
    request["method"] = "reboot";          // 方法名
    request["id"] = currentRequestId;      // 请求唯一标识
    request["params"] = params;            // 参数对象

    Message msg("request", "sendRequestHandler", "writeRestartConfiguration",{{"request",request}});

    dataBridge->sendCommandToWorker(channelId,msg);

    restartid = currentRequestId;

    waitingDialogLabel->setText("正在设置客户端重启，请稍候...");
    waitingDialog->show();
}

void MainWidget::handleSetResult_true(QJsonObject request, const QString& tip)
{
    int id = request.value("id").toInt();
    // 弹窗提示
    QMessageBox::information(this, "操作成功", QString("%1（ID: %2）").arg(tip).arg(id));

    clearID(); //清除id 防备逻辑污染
}



void MainWidget::handleSetResult_error(QJsonObject request,qintptr socketDescriptor){
    //通过按钮名称获得通道对应的名称
    QAbstractButton* btn = buttonGroup->button(socketDescriptor);
    QString name;
    if (btn) {
        name = btn->text();
    }
    // 创建弹窗用于显示结果
    QDialog* dialog = new QDialog(this);
    dialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // dialog->setWindowFlags(Qt::CustomizeWindowHint); //去掉右上角？
    dialog->setWindowTitle(name+" （错误响应）");
    QVBoxLayout* layout = new QVBoxLayout(dialog);

    // 显示原始 ID
    int id = request.value("id").toInt();
    layout->addWidget(new QLabel("请求 ID: " + QString::number(id)));

    QMap<int, QString> errortexts = {
        {uartid, "读取串口配置error"},
        {uartid_2, "下发串口配置error"},
        {canid, "读取can口配置error"},
        {canid_2, "下发can配置error"},
        {canfilterid, "读取can滤波字组error"},
        {canidfilter_2, "下发can滤波字组error"},
        {tcpid, "读取tcp配置error"},
        {tcpid_2, "下发tcp配置error"},
        {datetimeid, "对时设置error"},
        {restartid, "重启指令error"}
    };
    QString errortext = errortexts.value(id, "未知错误响应项");
    layout->addWidget(new QLabel("错误响应: " + errortext));

    QJsonObject errorObj = request["error"].toObject();

    int code = errorObj.value("code").toInt();
    QString code_str;
    switch (code) {
    case -32700:
        code_str = "  JSON 解析错误";
        break;
    case -32600:
        code_str = "  无效请求（格式错误）";
        break;
    case -32601:
        code_str = "  方法不存在";
        break;
    case -32602:
        code_str = "  无效参数";
        break;
    case -32603:
        code_str = "  内部错误";
        break;
    default:
        if(code>=-32099&&code<=-32000){
            code_str = "  服务器自定义错误";
        }
        break;
    }
    QString message = errorObj.value("message").toString();
    layout->addWidget(new QLabel("错误码: " + QString::number(code)+code_str));
    layout->addWidget(new QLabel("错误信息: " + message));

    QPushButton* btnOk = new QPushButton("确认", dialog);
    layout->addWidget(btnOk);
    connect(btnOk, &QPushButton::clicked, dialog, &QDialog::accept);

    dialog->setLayout(layout);
    dialog->exec();  // 模态显示

    clearID(); //清除所有id 防止逻辑污染
}

void MainWidget::setErrorToService(int id, int errorCode,QString errorDescription){
    QVariantMap params;
    params["id"] = id;
    params["errorCode"] = errorCode;
    params["errorDescription"] = errorDescription;

    Message msg("response", "sendResponseHandler", "uiResponseError", params);

    dataBridge->sendCommandToWorker(channelId, msg);
}

