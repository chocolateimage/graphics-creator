#include "color_button.hpp"
#include "color_picker.hpp"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOptionButton>
#include <QStylePainter>

ColorButton::ColorButton(const QColor &color, QWidget *parent)
    : QPushButton(parent) {
    setColor(color);
    setFixedHeight(24);
    connect(this, &ColorButton::clicked, this, &ColorButton::openColorDialog);
}

void ColorButton::openColorDialog() {
    ColorPickerDialog *dialog = new ColorPickerDialog(currentColor, this);
    if (dialog->exec() != QDialog::Accepted) {
        return;
    }
    setColor(dialog->currentColor);
}

void ColorButton::setColor(const QColor &color) {
    currentColor = color;
    emit colorChanged(color);
}

QColor ColorButton::color() const { return currentColor; }

void ColorButton::paintEvent(QPaintEvent *) {
    QStyleOptionButton option;
    initStyleOption(&option);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QBrush color = currentColor;
    painter.setPen(QPen(QColor(128, 128, 128), 1));
    if (option.state & QStyle::State_Sunken) {
        color = currentColor.darker(150);
    } else if (option.state & QStyle::State_MouseOver) {
        color = currentColor.darker(120);
    }
    painter.setBrush(color);
    painter.drawRect(rect().adjusted(1, 1, -1, -1));
}
