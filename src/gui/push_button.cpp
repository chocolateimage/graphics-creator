#include "push_button.hpp"
#include <QMouseEvent>

PushButton::PushButton(QWidget *parent) : QPushButton(parent) {}

void PushButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        emit rightClicked();
    }
    QPushButton::mousePressEvent(event);
}
