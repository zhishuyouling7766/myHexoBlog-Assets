#include "customcombobox_roundpopupandshadow.h"

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

CustomComboBox_roundPopupAndShadow::CustomComboBox_roundPopupAndShadow(QWidget *parent)
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

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);             // 模糊程度
    shadow->setOffset(3, 3);               // 向右下偏移 px
    shadow->setColor(QColor(0, 0, 0, 70)); // 半透明黑
    listView->setGraphicsEffect(shadow);



    // 设置最大可见项目数
    this->setMaxVisibleItems(10);


    // 设置 QComboBox 和 QListView 字体大小
    QFont font = this->font();
    font.setPointSize(12);
    listView->setFont(font);
}

CustomComboBox_roundPopupAndShadow::~CustomComboBox_roundPopupAndShadow(){
    delete listView;
}

//Popup是QComboBox弹出下拉区域本质的Widget，listView是放置在Popup内的控件
void CustomComboBox_roundPopupAndShadow::showPopup()
{
    QComboBox::showPopup();

    QWidget *popup = this->findChild<QFrame*>();
    popup->move(popup->x(), popup->y() + 4);


    QLayout *layout = popup->layout();  // 获取widget的布局
    if (layout) {
        int left, top, right, bottom;
        layout->getContentsMargins(&left, &top, &right, &bottom);  // 获取当前边距
        layout->setContentsMargins(left, top, 4, 4);  // 设置新的边距
    }

    QSize originalSize = popup->size();  // 获取当前大小
    int newWidth = originalSize.width() + 4;
    int newHeight = originalSize.height() + 4;

    popup->resize(newWidth, newHeight);  // 设置新大小

}
