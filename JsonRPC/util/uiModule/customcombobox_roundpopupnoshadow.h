#ifndef CUSTOMCOMBOBOX_ROUNDPOPUPNOSHADOW_H
#define CUSTOMCOMBOBOX_ROUNDPOPUPNOSHADOW_H

#include <QComboBox>
#include <QListView>



//下拉框圆角，无阴影样式
//可显示正常的QT自带弹出动画。该动画其实附带部分阴影，算是可以忍受的小bug

class CustomComboBox_roundPopupNoShadow: public QComboBox
{
    Q_OBJECT

public:
    explicit CustomComboBox_roundPopupNoShadow(QWidget *parent = nullptr);
    ~CustomComboBox_roundPopupNoShadow();

protected:
    void showPopup() override;

private:
    QListView *listView;
};

#endif // CUSTOMCOMBOBOX_ROUNDPOPUPNOSHADOW_H
