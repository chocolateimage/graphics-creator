#include "image_viewer.hpp"
#include <QMatrix4x4>
#include <QPaintEvent>
#include <QPainter>

ImageViewer::ImageViewer(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WidgetAttribute::WA_MouseTracking);
}

void ImageViewer::updateImage(QImage img) {
    image = std::move(img);
    clampMovePos();
    update();
}

void ImageViewer::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    if (image.isNull()) {
        return;
    }

    QRectF fitRect = fittedRect();
    bool smooth = zoom < 1.5f;

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawRect(rect());

    painter.translate(width() / 2.f, height() / 2.f);
    painter.scale(zoom, zoom);
    painter.translate(width() / -2.f, height() / -2.f);
    painter.translate(movePos);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRect(fitRect);
    constexpr int checkerboardSize = 8;
    for (qreal y = 0; y < fitRect.height(); y += checkerboardSize) {
        for (qreal x = 0; x < fitRect.width(); x += checkerboardSize) {
            int x2 = x / checkerboardSize;
            int y2 = y / checkerboardSize;
            bool isChecked = (x2 + y2) % 2 == 0;
            if (isChecked) {
                painter.setBrush(QColor(200, 200, 200));
                painter.drawRect(
                    x + fitRect.x(), y + fitRect.y(),
                    std::min(x + checkerboardSize, fitRect.width()) - x,
                    std::min(y + checkerboardSize, fitRect.height()) - y);
            }
        }
    }

    // TODO: scale image using the pixmap scale maybe?
    painter.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform, smooth);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, smooth);
    painter.drawImage(fittedRect(), image);

    if (isPicking) {
        painter.setRenderHint(QPainter::RenderHint::Antialiasing, false);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 0, 0, 200));

        painter.drawRect(QRectF(
            ((float)pickPosition.x() / image.width() * fitRect.width()) +
                fitRect.x(),
            ((float)pickPosition.y() / image.height() * fitRect.height()) +
                fitRect.y(),
            (float)fitRect.width() / image.width(),
            (float)fitRect.height() / image.height()));
    }

    painter.resetTransform();

    if (isPicking) {
        painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 180));

        QFont font;
        font.setWeight(QFont::Medium);
        font.setPixelSize(18);
        painter.setFont(font);

        int rectWidth = 350;
        int rectHeight = 64;
        int rectX = width() / 2 - rectWidth / 2;
        int rectY = 64;
        painter.drawRoundedRect(rectX, rectY, rectWidth, rectHeight, 8, 8);
        painter.setPen(Qt::white);
        painter.setBrush(Qt::NoBrush);
        painter.drawText(rectX, rectY, rectWidth, rectHeight / 2,
                         Qt::AlignCenter, pickText);

        QFont font2;
        font2.setPixelSize(14);
        painter.setFont(font2);

        painter.drawText(rectX, rectY + (rectHeight / 2), rectWidth,
                         rectHeight / 2, Qt::AlignCenter | Qt::TextWordWrap,
                         "Left click to confirm. Right click to cancel.");
    }
}

QRectF ImageViewer::fittedRect() {
    float w = width();
    float h = height();
    float verticalScale = h / image.height();
    float horizontalScale = w / image.width();
    float scale = std::min(verticalScale, horizontalScale);

    float newWidth = image.width() * scale;
    float newHeight = image.height() * scale;

    return {
        width() / 2.f - newWidth / 2.f,
        height() / 2.f - newHeight / 2.f,
        newWidth,
        newHeight,
    };
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event) {
    QPointF current = event->position();

    if (isPicking) {
        QRectF fitRect = fittedRect();
        QPointF pos = current;
        pos -= {width() / 2.f, height() / 2.f};
        pos /= zoom;
        pos -= {width() / -2.f, height() / -2.f};
        pos -= movePos;
        pos -= fitRect.topLeft();
        pos = QPointF(pos.x() / fitRect.width() * image.width(),
                      pos.y() / fitRect.height() * image.height());
        pickPosition = {qFloor(pos.x()), qFloor(pos.y())};

        update();
    }

    if (!dragging)
        return;
    if (zoom == 1)
        return;
    QPointF diff = current - lastDragMousePos;
    movePos += diff / zoom;

    clampMovePos();

    lastDragMousePos = current;

    update();
}

void ImageViewer::clampMovePos() {
    float xClamp = (fittedRect().width() - (fittedRect().width() / zoom)) / 2 -
                   (fittedRect().x() / zoom);
    float yClamp =
        (fittedRect().height() - (fittedRect().height() / zoom)) / 2 -
        (fittedRect().y() / zoom);
    bool xClamped{false};
    bool yClamped{false};
    if (movePos.x() > xClamp) {
        movePos.setX(xClamp);
        xClamped = true;
    }
    if (movePos.y() > yClamp) {
        movePos.setY(yClamp);
        yClamped = true;
    }
    if (movePos.x() < -xClamp) {
        if (xClamped) {
            movePos.setX(0);
        } else {
            movePos.setX(-xClamp);
        }
    }
    if (movePos.y() < -yClamp) {
        if (yClamped) {
            movePos.setY(0);
        } else {
            movePos.setY(-yClamp);
        }
    }
}

void ImageViewer::mousePressEvent(QMouseEvent *event) {

    if (isPicking && (event->button() == Qt::LeftButton ||
                      event->button() == Qt::RightButton)) {
        isPicking = false;
        update();
        updateCursor();
        if (event->button() == Qt::LeftButton) {
            emit pixelPicked(pickId, pickPosition);
        }
        return;
    }

    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::MiddleButton) {
        if (!dragging && zoom != 1) {
            dragging = true;
            lastDragMousePos = event->position();
            QGuiApplication::setOverrideCursor(
                Qt::CursorShape::ClosedHandCursor);
        }
    }

    // TODO: double click to fullscreen
    /*
        --- this seems to work, though setParent() could cause issues with
        memory leaks ---

        setParent(nullptr); show();
        setScreen(qApp->primaryScreen());
        showFullScreen();
    */
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::MiddleButton) {
        if (dragging) {
            dragging = false;
            QGuiApplication::restoreOverrideCursor();
        }
    }
}

void ImageViewer::wheelEvent(QWheelEvent *event) {
    // TODO: zoom around the cursor point instead of the middle
    float zoomScale = event->angleDelta().y() / 120.f;
    zoom = std::clamp(zoom * ((zoomScale * 0.2f) + 1), 1.f, 100.f);
    if (zoom == 1) {
        movePos = {0, 0};
        updateCursor();
    } else {
        updateCursor();
        clampMovePos();
    }
    update();
}

void ImageViewer::resizeEvent(QResizeEvent *event) {
    clampMovePos();
    update();
    QWidget::resizeEvent(event);
}

void ImageViewer::beginPicking(const QString &id, const QString &infoText) {
    isPicking = true;
    pickId = id;
    pickText = infoText;
    updateCursor();
    update();
}

void ImageViewer::updateCursor() {
    if (isPicking) {
        setCursor(Qt::CursorShape::CrossCursor);
        return;
    }

    if (zoom == 1) {
        setCursor(Qt::CursorShape::ArrowCursor);
    } else {
        setCursor(Qt::CursorShape::OpenHandCursor);
    }
}
