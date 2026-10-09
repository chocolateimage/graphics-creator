#include "color_button.hpp"
#include "color_picker.hpp"
#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
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

void ColorButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        QMenu menu;
        QMenu *copyMenu = menu.addMenu(QIcon::fromTheme("edit-copy"), "Copy");
        QAction *rgb = copyMenu->addAction("RGB");
        QAction *rgba = copyMenu->addAction("RGBA");
        QAction *hexRgb = copyMenu->addAction("HEX (#RGB)");
        QAction *hexRgba = copyMenu->addAction("HEX (#RGBA)");
        QAction *finalAction = menu.exec(QCursor::pos());
        QString copyString;
        if (finalAction == rgb) {
            copyString =
                QStringLiteral("rgb(") + QString::number(currentColor.red()) +
                QStringLiteral(",") + QString::number(currentColor.green()) +
                QStringLiteral(",") + QString::number(currentColor.blue()) +
                QStringLiteral(")");
        } else if (finalAction == rgba) {
            copyString =
                QStringLiteral("rgba(") + QString::number(currentColor.red()) +
                QStringLiteral(",") + QString::number(currentColor.green()) +
                QStringLiteral(",") + QString::number(currentColor.blue()) +
                QStringLiteral(",") + QString::number(currentColor.alpha()) +
                QStringLiteral(")");
        } else if (finalAction == hexRgb) {
            copyString = currentColor.name(QColor::NameFormat::HexRgb);
        } else if (finalAction == hexRgba) {
            copyString =
                QStringLiteral("#") +
                QString::number(currentColor.red(), 16).rightJustified(2, '0') +
                QString::number(currentColor.green(), 16)
                    .rightJustified(2, '0') +
                QString::number(currentColor.blue(), 16)
                    .rightJustified(2, '0') +
                QString::number(currentColor.alpha(), 16)
                    .rightJustified(2, '0');
        }

        if (!copyString.isEmpty()) {
            QClipboard *clipboad = QApplication::clipboard();
            clipboad->setText(copyString);
        }
    }
    QPushButton::mousePressEvent(event);
}
