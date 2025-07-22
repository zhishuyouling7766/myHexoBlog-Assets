#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("JsonRPC");

    initialPage = new initialWidget(this);
    mainWidget = new MainWidget(this,ui->statusbar);

    ui->stackedWidget->addWidget(initialPage);
    ui->stackedWidget->addWidget(mainWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete initialPage;
}

void MainWindow::setTextForStatusbar(QString text){
    ui->statusbar->showMessage(text);  // 或其它状态栏处理
}


void MainWindow::stackedWidgetSetCurrentIndex(int index){
    if(index<2&&index>=0){
        ui->stackedWidget->setCurrentIndex(index);
    }
}
