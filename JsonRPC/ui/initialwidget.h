#ifndef INITIALWIDGET_H
#define INITIALWIDGET_H

#include <QWidget>

namespace Ui {
class initialWidget;
}

class initialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit initialWidget(QWidget *parent = nullptr);
    ~initialWidget();

private slots:
    void on_pushButton_clicked();

private:
    QString getLocalIPv4();

private:
    Ui::initialWidget *ui;
};

#endif // INITIALWIDGET_H
