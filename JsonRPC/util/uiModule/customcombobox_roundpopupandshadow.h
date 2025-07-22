#ifndef CUSTOMCOMBOBOX_ROUNDPOPUPANDSHADOW_H
#define CUSTOMCOMBOBOX_ROUNDPOPUPANDSHADOW_H

#include <QComboBox>
#include <QListView>



//下拉框圆角，附带相应阴影样式
//！！！！！！附带阴影样式，会与QT原生动画产生冲突，动画期间Popup内边框会出现黑区，需要禁用动画：
    //QApplication::setEffectEnabled(Qt::UI_AnimateCombo, false); //禁用QComboBox弹出动画效果

class CustomComboBox_roundPopupAndShadow: public QComboBox
{
    Q_OBJECT

public:
    explicit CustomComboBox_roundPopupAndShadow(QWidget *parent = nullptr);
    ~CustomComboBox_roundPopupAndShadow();

protected:
    void showPopup() override;

private:
    QListView *listView;
};

#endif // CUSTOMCOMBOBOX_ROUNDPOPUPANDSHADOW_H
