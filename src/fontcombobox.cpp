#include "fontcombobox.hpp"
#include <QPainter>
#include <QStylePainter>

FontComboBox::FontComboBox(QWidget *parent) : QComboBox(parent) {
    addItems({"Hello", "World"});
};

void FontComboBox::showPopup() {
    qInfo() << "show";

    if (!popupWindow) {
        popupWindow = new FontComboBoxPopup();
        popupWindow->setWindowFlag(Qt::WindowType::Popup);
        popupWindow->setWindowFlag(Qt::WindowType::FramelessWindowHint);

        auto lay1 = new QVBoxLayout(popupWindow);

        auto scroll = new QScrollArea(popupWindow);
        auto content = new QWidget();
        scroll->setWidget(content);
        scroll->setWidgetResizable(true);
        auto lay = new QVBoxLayout(content);
        lay->setSpacing(32);
        for (int i = 0; i < 15; i++) {
            lay->addWidget(new QPushButton("Hello", content));
        }

        lay1->addWidget(scroll);
    }

    popupWindow->move(mapToGlobal(rect().bottomLeft()));
    popupWindow->resize(width(), 500);
    popupWindow->show();
}

void FontComboBox::hidePopup() {
    qInfo() << "hide";
    if (popupWindow) {
        popupWindow->close();
        delete popupWindow;
    }
}

void FontComboBoxPopup::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);
}
