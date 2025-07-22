#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "initialwidget.h"
#include "mainwidget.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void setTextForStatusbar(QString text);
    void stackedWidgetSetCurrentIndex(int index);


public:
    Ui::MainWindow *ui;

public:
    initialWidget *initialPage;
    MainWidget *mainWidget;
};
#endif // MAINWINDOW_H
