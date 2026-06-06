#include "image_viewer.hpp"
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QMatrix4x4>
#include <QPaintEvent>
#include <QPainter>
#include <QToolButton>

TransparentCornerFrame::TransparentCornerFrame(QWidget *parent)
    : QFrame(parent) {
    setFrameShape(QFrame::Shape::StyledPanel);
    setFrameShadow(QFrame::Shadow::Sunken);
    setProperty("_breeze_force_frame", true);
    setCursor(Qt::CursorShape::ArrowCursor);
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.3);
    setGraphicsEffect(opacityEffect);
}

void TransparentCornerFrame::enterEvent(QEnterEvent *event) {
    opacityEffect->setOpacity(1);
}

void TransparentCornerFrame::leaveEvent(QEvent *event) {
    opacityEffect->setOpacity(0.3);
}

ImageViewer::ImageViewer(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WidgetAttribute::WA_MouseTracking);
    setWindowTitle("Preview");

    auto lay = new QHBoxLayout(this);
    lay->setAlignment(Qt::AlignmentFlag::AlignTop |
                      Qt::AlignmentFlag::AlignRight);
    auto frame = new TransparentCornerFrame(this);

    auto frameLay = new QVBoxLayout(frame);
    windowButton = new QToolButton(frame);
    windowButton->setIcon(QIcon::fromTheme("view-fullscreen"));
    windowButton->setToolTip("Pop out window");
    connect(windowButton, &QToolButton::clicked, this, [this]() {
        poppedOut = !poppedOut;
        QPoint globalPos = mapToGlobal(QPoint(0, 0));
        setWindowFlag(Qt::WindowType::Window, poppedOut);
        if (poppedOut) {
            windowButton->setIcon(QIcon::fromTheme("view-restore"));
            windowButton->setToolTip("Pop in");
        } else {
            windowButton->setIcon(QIcon::fromTheme("view-fullscreen"));
            windowButton->setToolTip("Pop out window");
        }
        move(globalPos);
        show();
    });
    frameLay->addWidget(windowButton);

    auto darkCheckerboardButton = new QToolButton(frame);
    darkCheckerboardButton->setIcon(QIcon::fromTheme("composite-track-on"));
    darkCheckerboardButton->setToolTip("Dark checkboard background");
    darkCheckerboardButton->setCheckable(true);
    connect(darkCheckerboardButton, &QToolButton::toggled, this,
            [this](bool checked) {
                darkCheckerboard = checked;
                update();
            });
    frameLay->addWidget(darkCheckerboardButton);
    lay->addWidget(frame);
}

void ImageViewer::closeEvent(QCloseEvent *event) {
    event->ignore();
    windowButton->click();
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

    constexpr int checkerboardSize = 8;
    QPixmap checkerboardPattern(checkerboardSize * 2, checkerboardSize * 2);
    QPainter checkerboardPainter(&checkerboardPattern);
    QColor checkerboard1 =
        darkCheckerboard ? QColor(60, 60, 60) : QColor(200, 200, 200);
    QColor checkerboard2 = darkCheckerboard ? QColor(30, 30, 30) : Qt::white;
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
    painter.fillRect(fitRect, QBrush(checkerboardPattern));

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
    float zoomScale = event->angleDelta().y() / 120.f;

    QPointF cursorCentered =
        event->position() - QPointF(width() / 2.f, height() / 2.f);
    QPointF beforeZoomPoint = cursorCentered / zoom - movePos;

    zoom = std::clamp(zoom * ((zoomScale * 0.2f) + 1), 1.f, 100.f);
    if (zoom == 1) {
        movePos = {0, 0};
    } else {
        movePos = cursorCentered / zoom - beforeZoomPoint;
        clampMovePos();
    }
    updateCursor();
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
