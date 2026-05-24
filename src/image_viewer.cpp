#include "image_viewer.hpp"
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

    QRect fitRect = fittedRect().toRect();
    bool smooth = zoom < 1.5f;
    painter.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform, smooth);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing, smooth);

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
    for (int y = 0; y < fitRect.height(); y += checkerboardSize) {
        for (int x = 0; x < fitRect.width(); x += checkerboardSize) {
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
    painter.drawImage(fitRect, image);
    painter.resetTransform();
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
    if (!dragging)
        return;
    if (zoom == 1)
        return;
    QPointF current = event->position();
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
        setCursor(Qt::CursorShape::ArrowCursor);
    } else {
        setCursor(Qt::CursorShape::OpenHandCursor);
        clampMovePos();
    }
    update();
}

void ImageViewer::resizeEvent(QResizeEvent *event) {
    clampMovePos();
    update();
    QWidget::resizeEvent(event);
}
