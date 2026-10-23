#include "color_button.hpp"
#include "color_picker.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QHBoxLayout>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionButton>
#include <QStylePainter>

ColorButton::ColorButton(const QColor &color, QWidget *parent)
    : QPushButton(parent) {
    setAcceptDrops(true);
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

QColor ColorButton::colorFromMime(const QMimeData *mimeData) {
    if (mimeData->hasColor()) {
        return mimeData->colorData().value<QColor>();
    } else if (mimeData->hasText()) {
        QString text = mimeData->text().trimmed();
        if (text.startsWith("#") && text.length() == 7) {
            return QColor(text);
        }
    }

    return QColor::Invalid;
}

QString ColorButton::hexTransparency() {
    return QStringLiteral("#") +
           QString::number(currentColor.red(), 16).rightJustified(2, '0') +
           QString::number(currentColor.green(), 16).rightJustified(2, '0') +
           QString::number(currentColor.blue(), 16).rightJustified(2, '0') +
           QString::number(currentColor.alpha(), 16).rightJustified(2, '0');
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

    if (currentColor.alphaF() == 1) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(currentColor);
        painter.drawRect(1, 1, width() - 1, height() - 2);
    } else {
        QColor opaqueColor = currentColor;
        opaqueColor.setAlphaF(1);
        painter.setPen(Qt::NoPen);
        painter.setBrush(opaqueColor);
        painter.drawRect(1, 1, width() / 2, height() - 2);

        constexpr int checkerboardSize = 8;
        QPixmap checkerboardPattern(checkerboardSize * 2, checkerboardSize * 2);
        QPainter checkerboardPainter(&checkerboardPattern);
        QColor checkerboard1 = QColor(200, 200, 200);
        QColor checkerboard2 = Qt::white;
        checkerboardPainter.fillRect(0, 0, checkerboardSize, checkerboardSize,
                                     checkerboard1);
        checkerboardPainter.fillRect(checkerboardSize, checkerboardSize,
                                     checkerboardSize, checkerboardSize,
                                     checkerboard1);
        checkerboardPainter.fillRect(0, checkerboardSize, checkerboardSize,
                                     checkerboardSize, checkerboard2);
        checkerboardPainter.fillRect(checkerboardSize, 0, checkerboardSize,
                                     checkerboardSize, checkerboard2);
        checkerboardPainter.end();
        painter.fillRect(width() / 2, 1, width() / 2, height() - 2,
                         QBrush(checkerboardPattern));

        painter.setPen(Qt::NoPen);
        painter.setBrush(currentColor);
        painter.drawRect(width() / 2, 1, width() / 2, height() - 2);
    }

    painter.setPen(QPen(QColor(128, 128, 128), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(1, 1, width() - 2, height() - 2);

    QColor overlayColor = Qt::transparent;
    if (option.state & QStyle::State_Sunken) {
        overlayColor = QColor(0, 0, 0, 80);
    } else if (option.state & QStyle::State_MouseOver) {
        overlayColor = QColor(0, 0, 0, 50);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(overlayColor);
    painter.drawRect(rect());
}

void ColorButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->position();
    }
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
            copyString = hexTransparency();
        }

        if (!copyString.isEmpty()) {
            QClipboard *clipboad = QApplication::clipboard();
            clipboad->setText(copyString);
        }
    }
    QPushButton::mousePressEvent(event);
}

void ColorButton::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        if ((event->position() - dragStartPosition).manhattanLength() >=
            QApplication::startDragDistance()) {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData();
            mimeData->setColorData(currentColor);
            if (currentColor.alphaF() == 1) {
                mimeData->setText(currentColor.name());
            } else {
                mimeData->setText(hexTransparency());
            }
            drag->setMimeData(mimeData);

            QPixmap previewPix(24, 24);
            previewPix.fill(currentColor);
            drag->setPixmap(previewPix);

            drag->exec(Qt::DropAction::CopyAction);
        }
    }

    QPushButton::mouseMoveEvent(event);
}

void ColorButton::dragEnterEvent(QDragEnterEvent *event) {
    QColor color = colorFromMime(event->mimeData());
    if (color.isValid()) {
        event->accept();
    }
}

void ColorButton::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    QColor color = colorFromMime(event->mimeData());
    setColor(color);
}
