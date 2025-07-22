#include "customcombobox_roundpopupnoshadow.h"

#include <QLineEdit>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QTimer>

//下列样式如需要，须在使用该控件处声明，此处声明会出现奇奇怪怪的bug
// QComboBox
// {
// border:1px solid #2a2a2a;
//     border-radius: 5px;
// }
// QComboBox::drop-down {
//     subcontrol-origin: padding;
//     subcontrol-position: center right;
//     left: -5px;
//     width: 10px;
//     border: none;
//     image: url(:/resource/image/jiantou_down.png);
// }


CustomComboBox_roundPopupNoShadow::CustomComboBox_roundPopupNoShadow(QWidget *parent)
    : QComboBox(parent) {
    // 创建一个 QListView
    listView = new QListView(nullptr);

    // 设置 QListView 为 QComboBox 的视图
    setView(listView);

    //相当于显式声明，控件需要启用QSS样式！该句话非常重要！
    setStyleSheet(R"(
        QComboBox {
        }
    )");

    // 设置 QComboBox 的样式表
    listView->setStyleSheet(R"(
        QListView {
            border: 1px solid #3b3b3b;
            border-radius: 5px;
            padding: 5px;
            background-color: white;
        }
        QAbstractItemView::item:selected {
            background: rgba(0, 0, 0, 7%);
            color: #000000;
            border-radius: 3px;
        }
    )");




    // 您可以根据需要自定义 QListView 的其他属性
    listView->setSpacing(2); // 设置项间距
    listView->setUniformItemSizes(true); // 设置统一项大小



    QWidget* popup = view()->window();

    popup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);  // 保证透明区域可见
    popup->show();


    // 设置最大可见项目数
    this->setMaxVisibleItems(10);


    // 设置 QComboBox 和 QListView 字体大小
    QFont font = this->font();
    font.setPointSize(12);
    listView->setFont(font);
}

CustomComboBox_roundPopupNoShadow::~CustomComboBox_roundPopupNoShadow(){
    delete listView;
}

//Popup是QComboBox弹出下拉区域本质的Widget，listView是放置在Popup内的控件
void CustomComboBox_roundPopupNoShadow::showPopup()
{
    QComboBox::showPopup();

    QWidget *popup = this->findChild<QFrame*>();
    popup->move(popup->x(), popup->y() + 4);


}
