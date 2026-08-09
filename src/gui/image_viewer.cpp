#include "image_viewer.hpp"
#include "animatable/element/group_element.hpp"
#include "animatable/element/image_element.hpp"
#include "gui.hpp"
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QMatrix4x4>
#include <QMenu>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPaintEvent>
#include <QPainter>
#include <QToolButton>
#include <QToolTip>

constexpr int RESIZE_HANDLE_SIZE = 8;

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

ImageViewer::ImageViewer(Scene *scene, QWidget *parent)
    : QWidget(parent), scene(scene) {
    setAttribute(Qt::WidgetAttribute::WA_MouseTracking);
    setAttribute(Qt::WidgetAttribute::WA_InputMethodEnabled);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setAcceptDrops(true);
    setWindowTitle("Preview");

    // top left
    resizeModes.push_back({
        .moveX = 1,
        .moveY = 1,
        .sizeX = -1,
        .sizeY = -1,
        .sideX = 0,
        .sideY = 0,
        .cursor = Qt::CursorShape::SizeFDiagCursor,
    });
    // top
    resizeModes.push_back({
        .moveX = 0,
        .moveY = 1,
        .sizeX = 0,
        .sizeY = -1,
        .sideX = .5,
        .sideY = 0,
        .cursor = Qt::CursorShape::SizeVerCursor,
    });
    // top right
    resizeModes.push_back({
        .moveX = 0,
        .moveY = 1,
        .sizeX = 1,
        .sizeY = -1,
        .sideX = 1,
        .sideY = 0,
        .cursor = Qt::CursorShape::SizeBDiagCursor,
    });
    // right
    resizeModes.push_back({
        .moveX = 0,
        .moveY = 0,
        .sizeX = 1,
        .sizeY = 0,
        .sideX = 1,
        .sideY = 0.5,
        .cursor = Qt::CursorShape::SizeHorCursor,
    });
    // bottom right
    resizeModes.push_back({
        .moveX = 0,
        .moveY = 0,
        .sizeX = 1,
        .sizeY = 1,
        .sideX = 1,
        .sideY = 1,
        .cursor = Qt::CursorShape::SizeFDiagCursor,
    });
    // bottom
    resizeModes.push_back({
        .moveX = 0,
        .moveY = 0,
        .sizeX = 0,
        .sizeY = 1,
        .sideX = .5,
        .sideY = 1,
        .cursor = Qt::CursorShape::SizeVerCursor,
    });
    // bottom left
    resizeModes.push_back({
        .moveX = 1,
        .moveY = 0,
        .sizeX = -1,
        .sizeY = 1,
        .sideX = 0,
        .sideY = 1,
        .cursor = Qt::CursorShape::SizeBDiagCursor,
    });
    // left
    resizeModes.push_back({
        .moveX = 1,
        .moveY = 0,
        .sizeX = -1,
        .sizeY = 0,
        .sideX = 0,
        .sideY = .5,
        .cursor = Qt::CursorShape::SizeHorCursor,
    });

    auto lay = new QHBoxLayout(this);
    lay->setAlignment(Qt::AlignmentFlag::AlignTop |
                      Qt::AlignmentFlag::AlignRight);
    cornerFrame = new TransparentCornerFrame(this);
    cornerFrame->hide();

    auto frameLay = new QVBoxLayout(cornerFrame);

    auto darkCheckerboardButton = new QToolButton(cornerFrame);
    darkCheckerboardButton->setIcon(QIcon::fromTheme("composite-track-on"));
    darkCheckerboardButton->setToolTip("Dark checkboard background");
    darkCheckerboardButton->setCheckable(true);
    darkCheckerboardButton->setChecked(true);
    connect(darkCheckerboardButton, &QToolButton::toggled, this,
            [this](bool checked) {
                darkCheckerboard = checked;
                update();
            });
    frameLay->addWidget(darkCheckerboardButton);

    auto saveFrameButton = new QToolButton(cornerFrame);
    saveFrameButton->setIcon(QIcon::fromTheme("document-save"));
    saveFrameButton->setToolTip("Save frame…");
    connect(saveFrameButton, &QToolButton::clicked, this, [this]() {
        QMenu menu;
        QAction *fileAction = menu.addAction("To file…");
        QAction *clipboardAction = menu.addAction("To clipboard");
        QAction *selectedAction = menu.exec(QCursor::pos());
        if (selectedAction == fileAction) {
            QString path = QFileDialog::getSaveFileName(
                this, "Save frame", QString(),
                "PNG image (*.png);;All files (*.*)");
            if (path.isEmpty())
                return;

            image.save(path);
        } else if (selectedAction == clipboardAction) {
            QClipboard *clipboad = QApplication::clipboard();
            clipboad->setImage(image);
        }
    });
    frameLay->addWidget(saveFrameButton);
    lay->addWidget(cornerFrame);

    if (scene) {
        connect(scene, &Scene::elementSelectionChanged, this,
                &ImageViewer::elementSelectionChanged);
        connect(scene, &Scene::elementEditModeChanged, this,
                &ImageViewer::elementEditModeChanged);
        connect(scene, &Scene::playbackStateChanged, this,
                &ImageViewer::playbackStateChanged);
    }
}

void ImageViewer::elementEditModeChanged(Element *element, bool editMode) {
    if (textElementEditor != nullptr) {
        delete textElementEditor;
        textElementEditor = nullptr;
    }

    if (editMode) {
        TextElement *textElement = dynamic_cast<TextElement *>(element);
        if (textElement != nullptr) {
            textElementEditor =
                new TextElementEditor(mainWindow, scene, textElement, this);
            setFocus();
        }
    }

    update();
}

void ImageViewer::elementSelectionChanged(QList<Element *> elements) {
    update();
}

void ImageViewer::playbackStateChanged(bool playing) { update(); }

QPoint ImageViewer::getActualPickPosition() {
    if (startPickPosition.x() != -1 || startPickPosition.y() != -1) {
        if (QApplication::queryKeyboardModifiers().testFlag(
                Qt::ShiftModifier)) {
            QPoint fullSize = (pickPosition - startPickPosition);
            int size = std::max(fullSize.x(), fullSize.y());
            return startPickPosition + QPoint(size, size);
        }
    }
    return pickPosition;
}

QRect ImageViewer::getPickRect() {
    QRect rect;
    QPoint pickPosition = getActualPickPosition();
    if (startPickPosition.x() > pickPosition.x()) {
        rect.setX(pickPosition.x());
        rect.setWidth(startPickPosition.x() - pickPosition.x() + 1);
    } else {
        rect.setX(startPickPosition.x());
        rect.setWidth(pickPosition.x() - startPickPosition.x() + 1);
    }
    if (startPickPosition.y() > pickPosition.y()) {
        rect.setY(pickPosition.y());
        rect.setHeight(startPickPosition.y() - pickPosition.y() + 1);
    } else {
        rect.setY(startPickPosition.y());
        rect.setHeight(pickPosition.y() - startPickPosition.y() + 1);
    }
    return rect;
}

void ImageViewer::enterEvent(QEnterEvent *event) { cornerFrame->show(); }

void ImageViewer::leaveEvent(QEvent *event) { cornerFrame->hide(); }

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
        QPoint pickPosition = getActualPickPosition();
        if (startPickPosition.x() == -1 && startPickPosition.y() == -1) {
            painter.setRenderHint(QPainter::RenderHint::Antialiasing, false);
            painter.setPen(QPen(QColor(0, 0, 0, 255),
                                (float)fitRect.width() / image.width() * .1));
            painter.setBrush(QColor(255, 0, 0, 100));
            painter.drawRect(QRectF(
                ((float)pickPosition.x() / image.width() * fitRect.width()) +
                    fitRect.x(),
                ((float)pickPosition.y() / image.height() * fitRect.height()) +
                    fitRect.y(),
                (float)fitRect.width() / image.width(),
                (float)fitRect.height() / image.height()));
        } else {
            painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);
            QPen pen = QPen(palette().accent(),
                            (float)fitRect.width() / image.width());
            pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            float startX = ((float)startPickPosition.x() / image.width() *
                            fitRect.width()) +
                           fitRect.x();
            float startY = ((float)startPickPosition.y() / image.height() *
                            fitRect.height()) +
                           fitRect.y();
            float endX =
                ((float)pickPosition.x() / image.width() * fitRect.width()) +
                fitRect.x();
            float endY =
                ((float)pickPosition.y() / image.height() * fitRect.height()) +
                fitRect.y();
            painter.drawRect(QRectF(startX + pen.widthF() / 2.f,
                                    startY + pen.widthF() / 2.f, endX - startX,
                                    endY - startY));
        }
    }

    painter.resetTransform();

    if (isPicking && !pickId.isEmpty()) {
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

    float zoomElement =
        pixelToViewport({2, 2}).x() - pixelToViewport({1, 1}).x();
    if (scene && !scene->isPlaying()) {
        FrameInfo fi = {scene->currentFrame};

        if (!isMovingElements && hoverElement &&
            !scene->selectedElements.contains(hoverElement)) {
            QPen pen(palette().accent(), 2);
            pen.setStyle(Qt::PenStyle::DotLine);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);

            QRect boundingBox = hoverElement->getBoundingBox(fi);
            QPointF pos = boundingBox.topLeft();
            QPointF bottomRight = boundingBox.bottomRight() + QPoint{1, 1};
            pos = pixelToViewport(pos);
            QPointF size = pixelToViewport(bottomRight) - pos;
            painter.drawRect(pos.x(), pos.y(), size.x() + 1, size.y() + 1);
        }

        for (auto element : scene->selectedElements) {
            painter.setPen(QPen(palette().accent(), 2));
            painter.setBrush(Qt::NoBrush);

            QRect boundingBox = element->getBoundingBox(fi);
            QPointF pos = boundingBox.topLeft();
            QPointF bottomRight = boundingBox.bottomRight() + QPoint{1, 1};
            pos = pixelToViewport(pos);
            QPointF size = pixelToViewport(bottomRight) - pos;
            painter.drawRect(pos.x(), pos.y(), size.x() + 1, size.y() + 1);

            if (element->isResizable()) {
                painter.setPen(QPen(palette().accent().color().darker(), 1));
                painter.setBrush(palette().accent());
                for (const auto &resizeMode : resizeModes) {
                    QPointF resizePos =
                        pos + QPointF{size.x() * resizeMode.sideX,
                                      size.y() * resizeMode.sideY};
                    painter.drawRect(
                        resizePos.x() + RESIZE_HANDLE_SIZE / -2. + 1,
                        resizePos.y() + RESIZE_HANDLE_SIZE / -2. + 1,
                        RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE);
                }
            }

            if (element->editMode) {
                painter.save();
                painter.translate(pos);
                painter.scale(zoomElement, zoomElement);
                textElementEditor->paint(painter);
                painter.restore();
            }
        }
    }

    if (isDroppingImage) {
        QPointF pos = pixelToViewport(dropImageCursor);
        painter.save();
        painter.translate(pos);
        painter.scale(zoomElement, zoomElement);
        painter.drawImage(0, 0,
                          dropImageFitted
                              ? dropImagePreview.scaled(dropImageSize)
                              : dropImagePreview);
        painter.restore();
    }

    if (isMovingElements) {
        paintSnapVisualRect(painter, snapVisualRect1);
        paintSnapVisualRect(painter, snapVisualRect2);

        if (moveSpacing > 0 && snapElementRectPreview.isValid()) {
            QPointF topLeft = pixelToViewport(snapElementRectPreview.topLeft());
            QPointF bottomRight = pixelToViewport(
                snapElementRectPreview.bottomRight() + QPoint{1, 1});
            painter.setPen(QPen(QColor(61, 148, 255, 255), 2));
            painter.setBrush(QColor(61, 148, 255, 40));
            painter.drawRect(topLeft.x(), topLeft.y(),
                             bottomRight.x() - topLeft.x(),
                             bottomRight.y() - topLeft.y());
        }
    }
}

void ImageViewer::paintSnapVisualRect(QPainter &painter,
                                      const QRect &snapVisualRect) {
    if (!snapVisualRect.isValid())
        return;

    QPointF topLeft = pixelToViewport(snapVisualRect.topLeft());
    QPointF bottomRight =
        pixelToViewport(snapVisualRect.bottomRight() + QPoint{1, 1});
    float w = std::max(1., bottomRight.x() - topLeft.x());
    float h = std::max(1., bottomRight.y() - topLeft.y());
    if (snapVisualRect.width() == 1) {
        w = 2;
    }
    if (snapVisualRect.height() == 1) {
        h = 2;
    }
    painter.setPen(QPen(QColor(0, 0, 0, 80), .5));
    painter.setBrush(QColor(255, 255, 0, 80));
    painter.drawRect(topLeft.x(), topLeft.y(), w, h);
}

void ImageViewer::dragEnterEvent(QDragEnterEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    if (scene && mimeData->hasUrls() && !mimeData->urls().isEmpty()) {
        QUrl url = mimeData->urls().first();
        QMimeDatabase db;
        QMimeType type = db.mimeTypeForUrl(url);
        if (type.name().startsWith("image/")) {
            dropImagePath = url.toLocalFile();
            dropImagePreview = QImage(dropImagePath);
            dropImageCursor = viewportToPixel(event->position());
            dropImageSize = dropImagePreview.size();
            dropImageFitted = false;
            isDroppingImage = true;
            event->setDropAction(Qt::DropAction::LinkAction);
            event->accept();
            update();
        }
    }
}

void ImageViewer::dragMoveEvent(QDragMoveEvent *event) {
    if (isDroppingImage) {
        dropImageCursor = viewportToPixel(event->position());
        if (dropImageCursor.manhattanLength() < 50) {
            dropImageCursor = {0, 0};
            int w = scene->width;
            int h = scene->height;
            float verticalScale = (float)h / dropImagePreview.height();
            float horizontalScale = (float)w / dropImagePreview.width();
            float scale = std::max(verticalScale, horizontalScale);

            int newWidth = dropImagePreview.width() * scale;
            int newHeight = dropImagePreview.height() * scale;
            dropImageCursor = {w / 2 - newWidth / 2, h / 2 - newHeight / 2};
            dropImageSize = {newWidth, newHeight};
            dropImageFitted = true;
        } else {
            dropImageSize = dropImagePreview.size();
            dropImageFitted = false;
        }
        update();
    }
}

void ImageViewer::dragLeaveEvent(QDragLeaveEvent *event) {
    if (isDroppingImage) {
        isDroppingImage = false;
        dropImagePreview = QImage();
        update();
    }
}

void ImageViewer::dropEvent(QDropEvent *event) {
    if (isDroppingImage) {
        ImageElement *imageElement = new ImageElement();
        imageElement->x.set(dropImageCursor.x(), {0});
        imageElement->y.set(dropImageCursor.y(), {0});
        imageElement->w.set(dropImageSize.width(), {0});
        imageElement->h.set(dropImageSize.height(), {0});
        imageElement->path.set(dropImagePath.toStdString(), {0});
        imageElement->setObjectName(
            dropImagePath.split("/").last().split("\\").last());
        mainWindow->addElementUndoable(imageElement);
        dropImagePreview = QImage();
        isDroppingImage = false;
        update();
    }
}

QPointF ImageViewer::pixelToViewport(QPointF pos) {
    QRectF fitRect = fittedRect();
    pos = QPointF(pos.x() / image.width() * fitRect.width(),
                  pos.y() / image.height() * fitRect.height());
    pos += fitRect.topLeft();
    pos += movePos;
    pos += {width() / -2.f, height() / -2.f};
    pos *= zoom;
    pos += {width() / 2.f, height() / 2.f};
    return pos;
}

QPoint ImageViewer::viewportToPixel(QPointF pos) {
    QRectF fitRect = fittedRect();
    pos -= {width() / 2.f, height() / 2.f};
    pos /= zoom;
    pos -= {width() / -2.f, height() / -2.f};
    pos -= movePos;
    pos -= fitRect.topLeft();
    pos = QPointF(pos.x() / fitRect.width() * image.width(),
                  pos.y() / fitRect.height() * image.height());
    return {qFloor(pos.x()), qFloor(pos.y())};
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
    QPoint pixelPos = viewportToPixel(current);

    if (isPicking) {
        pickPosition = pixelPos;

        update();

        if (startPickPosition.x() != -1 || startPickPosition.y() != -1) {
            QRect pickRect = getPickRect();
            QToolTip::showText(QCursor::pos(),
                               QString::number(pickRect.width()) + "x" +
                                   QString::number(pickRect.height()));
        }
    }

    FrameInfo frameInfo{scene->currentFrame};

    if (activeResizeMode != -1) {
        ResizeMode &resizeMode = resizeModes[activeResizeMode];
        int resizeX = pixelPos.x() - startResizePosition.x();
        int resizeY = pixelPos.y() - startResizePosition.y();
        int targetW = resizeX * resizeMode.sizeX + startResizeRect.width();
        int targetH = resizeY * resizeMode.sizeY + startResizeRect.height();
        if (targetW < 1) {
            resizeX += targetW - 1;
        }
        if (targetH < 1) {
            resizeY += targetH - 1;
        }
        int targetX = resizeX * resizeMode.moveX + startResizeRect.x();
        int targetY = resizeY * resizeMode.moveY + startResizeRect.y();
        if (resizeElement->x.get(frameInfo) != targetX) {
            resizeElement->x.set(targetX, frameInfo);
        }
        if (resizeElement->y.get(frameInfo) != targetY) {
            resizeElement->y.set(targetY, frameInfo);
        }
        if (resizeElement->w.get(frameInfo) != targetW) {
            resizeElement->w.set(targetW, frameInfo);
        }
        if (resizeElement->h.get(frameInfo) != targetH) {
            resizeElement->h.set(targetH, frameInfo);
        }
        update();
        return;
    }

    if (isMovingElements) {
        if (moveSpacingMoving) {
            moveSpacing = std::max(
                0, moveSpacing + (QCursor::pos() - moveSpacingCursorStart).x());
            QCursor::setPos(moveSpacingCursorStart);
            pixelPos = viewportToPixel(mapFromGlobal(moveSpacingCursorStart));
        }

        QPoint diff = pixelPos - startMovePosition;
        if (QApplication::queryKeyboardModifiers().testFlag(
                Qt::ShiftModifier)) {
            if (std::abs(diff.x()) > std::abs(diff.y())) {
                diff = {diff.x(), 0};
            } else {
                diff = {0, diff.y()};
            }
        }

        QRect rect;
        int spacing = moveSpacing;

        bool first = true;
        int index = 0;
        for (auto element : scene->selectedElements) {
            QRect bbox = element->getBoundingBox(frameInfo);
            QRect thisRect = {startElementBoundPositions[index].x() + diff.x(),
                              startElementBoundPositions[index].y() + diff.y(),
                              bbox.width(), bbox.height()};
            if (index == 0) {
                rect = thisRect;
            } else {
                rect = rect.united(thisRect);
            }
            index++;
        }

        rect = rect.marginsAdded(QMargins{spacing, spacing, spacing, spacing});
        snapElementRectPreview = rect;

        QList<QRect> snapRects;
        if (!QApplication::queryKeyboardModifiers().testFlag(
                Qt::ControlModifier)) {
            snapRects.append({0, 0, scene->width, scene->height});
            for (auto element : scene->elements) {
                if (!element->visible)
                    continue;
                if (scene->selectedElements.contains(element))
                    continue;
                bool skip = false;
                for (auto selectedElement : scene->selectedElements) {
                    if (selectedElement->isAnyChild(element) ||
                        selectedElement->isAnyParent(element)) {
                        skip = true;
                        continue;
                    }
                }
                if (skip)
                    continue;

                snapRects.append(element->getBoundingBox(frameInfo));
            }
        } else {
            snapElementRectPreview = {};
        }

        int finalDiffX{0};
        int finalDiffY{0};
        int lastLengthX = INT32_MAX;
        int lastLengthY = INT32_MAX;

        float zoomElement =
            pixelToViewport({2, 2}).x() - pixelToViewport({1, 1}).x();
        int threshold = 12 / zoomElement;

        for (const auto &snapRect : snapRects) {
            if ((rect.y() + rect.height()) >= (snapRect.y() - threshold) &&
                rect.y() < (snapRect.y() + snapRect.height() + threshold)) {
                int outerLeftPos = snapRect.x() - rect.width();
                int outerLeftDist = qAbs(outerLeftPos - rect.x());

                int outerRightPos = snapRect.x() + snapRect.width();
                int outerRightDist = qAbs(outerRightPos - rect.x());

                int innerLeftPos = snapRect.x();
                int innerLeftDist = qAbs(innerLeftPos - rect.x());

                int innerRightPos =
                    snapRect.x() + snapRect.width() - rect.width();
                int innerRightDist = qAbs(innerRightPos - rect.x());

                int centerPos =
                    snapRect.x() + snapRect.width() / 2 - rect.width() / 2;
                int centerDist = qAbs(centerPos - rect.x());

                if (outerLeftDist < lastLengthX) {
                    lastLengthX = outerLeftDist;
                    finalDiffX = outerLeftPos;
                    snapVisualRect1 = {innerLeftPos, snapRect.y(), 1,
                                       snapRect.height()};
                }
                if (outerRightDist < lastLengthX) {
                    lastLengthX = outerRightDist;
                    finalDiffX = outerRightPos;
                    snapVisualRect1 = {outerRightPos, snapRect.y(), 1,
                                       snapRect.height()};
                }
                if (innerLeftDist < lastLengthX) {
                    lastLengthX = innerLeftDist;
                    finalDiffX = innerLeftPos;
                    snapVisualRect1 = {innerLeftPos, snapRect.y(), 1,
                                       snapRect.height()};
                }
                if (innerRightDist < lastLengthX) {
                    lastLengthX = innerRightDist;
                    finalDiffX = innerRightPos;
                    snapVisualRect1 = {outerRightPos, snapRect.y(), 1,
                                       snapRect.height()};
                }
                if (centerDist < lastLengthX) {
                    lastLengthX = centerDist;
                    finalDiffX = centerPos;
                    snapVisualRect1 = {snapRect.x() + snapRect.width() / 2,
                                       snapRect.y(), 1, snapRect.height()};
                }
            }

            if ((rect.x() + rect.width()) >= (snapRect.x() - threshold) &&
                rect.x() < (snapRect.x() + snapRect.width() + threshold)) {
                int outerLeftPos = snapRect.y() - rect.height();
                int outerLeftDist = qAbs(outerLeftPos - rect.y());

                int outerRightPos = snapRect.y() + snapRect.height();
                int outerRightDist = qAbs(outerRightPos - rect.y());

                int innerLeftPos = snapRect.y();
                int innerLeftDist = qAbs(innerLeftPos - rect.y());

                int innerRightPos =
                    snapRect.y() + snapRect.height() - rect.height();
                int innerRightDist = qAbs(innerRightPos - rect.y());

                int centerPos =
                    snapRect.y() + snapRect.height() / 2 - rect.height() / 2;
                int centerDist = qAbs(centerPos - rect.y());

                if (outerLeftDist < lastLengthY) {
                    lastLengthY = outerLeftDist;
                    finalDiffY = outerLeftPos;
                    snapVisualRect2 = {snapRect.x(), innerLeftPos,
                                       snapRect.width(), 1};
                }
                if (outerRightDist < lastLengthY) {
                    lastLengthY = outerRightDist;
                    finalDiffY = outerRightPos;
                    snapVisualRect2 = {snapRect.x(), outerRightPos,
                                       snapRect.width(), 1};
                }
                if (innerLeftDist < lastLengthY) {
                    lastLengthY = innerLeftDist;
                    finalDiffY = innerLeftPos;
                    snapVisualRect2 = {snapRect.x(), innerLeftPos,
                                       snapRect.width(), 1};
                }
                if (innerRightDist < lastLengthY) {
                    lastLengthY = innerRightDist;
                    finalDiffY = innerRightPos;
                    snapVisualRect2 = {snapRect.x(), outerRightPos,
                                       snapRect.width(), 1};
                }
                if (centerDist < lastLengthY) {
                    lastLengthY = centerDist;
                    finalDiffY = centerPos;
                    snapVisualRect2 = {snapRect.x(),
                                       snapRect.y() + snapRect.height() / 2,
                                       snapRect.width(), 1};
                }
            }
        }

        if (lastLengthX < threshold) {
            diff.setX(finalDiffX - rect.x() + diff.x());
            snapElementRectPreview =
                QRect(finalDiffX, snapElementRectPreview.y(),
                      snapElementRectPreview.width(),
                      snapElementRectPreview.height());
        } else {
            snapVisualRect1 = {0, 0, 0, 0};
        }
        if (lastLengthY < threshold) {
            diff.setY(finalDiffY - rect.y() + diff.y());
            snapElementRectPreview =
                QRect(snapElementRectPreview.x(), finalDiffY,
                      snapElementRectPreview.width(),
                      snapElementRectPreview.height());
        } else {
            snapVisualRect2 = {0, 0, 0, 0};
        }

        index = 0;
        for (auto element : scene->selectedElements) {
            int targetX = startElementPositions[index].x() + diff.x();
            int targetY = startElementPositions[index].y() + diff.y();
            if (element->x.get(frameInfo) != targetX) {
                element->x.set(targetX, frameInfo);
            }
            if (element->y.get(frameInfo) != targetY) {
                element->y.set(targetY, frameInfo);
            }
            index++;
        }

        didMove = true;

        update();

        return;
    }

    {
        bool exact =
            QApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier);

        hoverElement = nullptr;
        for (auto element : scene->elements) {
            if (!element->visible)
                continue;

            if (exact) {
                GroupElement *groupElement =
                    dynamic_cast<GroupElement *>(element);
                if (groupElement)
                    continue;
            } else {
                if (element->hasParent())
                    continue;
            }

            if (element->getBoundingBox(frameInfo).contains(pixelPos)) {
                hoverElement = element;
                break;
            }
        }

        if (!exact) {
            for (auto element : scene->selectedElements) {
                if (element->getBoundingBox(frameInfo).contains(pixelPos)) {
                    hoverElement = element;
                    break;
                }
            }
        }
        update();
    }

    hoverResizeMode = -1;
    hoverResizeElement = nullptr;
    for (auto element : scene->selectedElements) {
        if (!element->isResizable())
            continue;

        QRect boundingBox = element->getBoundingBox(frameInfo);
        QPointF pos = boundingBox.topLeft();
        QPointF bottomRight = boundingBox.bottomRight() + QPoint{1, 1};
        pos = pixelToViewport(pos);
        QPointF size = pixelToViewport(bottomRight) - pos;
        int index = 0;
        for (const auto &resizeMode : resizeModes) {
            QPointF resizePos =
                pos +
                QPointF{size.x() * resizeMode.sideX - RESIZE_HANDLE_SIZE / 2.,
                        size.y() * resizeMode.sideY - RESIZE_HANDLE_SIZE / 2.};
            QRectF resizeRect = QRect(resizePos.x(), resizePos.y(),
                                      RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE);
            if (resizeRect.contains(current)) {
                hoverResizeMode = index;
                hoverResizeElement = element;
                break;
            }
            index++;
        }
    }

    updateCursor();

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
    if (isPicking) {
        if (event->button() == Qt::LeftButton) {
            if (pickType == PickType::Point) {
                stopPicking();
                emit pixelPicked(pickId, getActualPickPosition());
            } else if (pickType == PickType::Rect) {
                startPickPosition = pickPosition;
                update();
            }

            return;
        } else if (event->button() == Qt::RightButton) {
            if (pickId.isEmpty())
                return;

            stopPicking();

            return;
        }
    }

    if (event->button() == Qt::RightButton) {
        if (isMovingElements) {
            moveSpacingCursorStart = QCursor::pos();
            moveSpacingMoving = true;
        }
    }

    if (event->button() == Qt::LeftButton) {
        if (scene && !scene->isPlaying()) {
            FrameInfo frameInfo = {scene->currentFrame};
            if (hoverResizeMode != -1) {
                startResizePosition = viewportToPixel(event->position());
                resizeElement = hoverResizeElement;
                resizeOldX = resizeElement->x.serialize();
                resizeOldY = resizeElement->y.serialize();
                resizeOldW = resizeElement->w.serialize();
                resizeOldH = resizeElement->h.serialize();
                startResizeRect = resizeElement->getRawBoundingBox(frameInfo);
                activeResizeMode = hoverResizeMode;
                return;
            }

            Element *clickedElement = hoverElement;
            startMovePosition = viewportToPixel(event->position());

            if (clickedElement) {
                isMovingElements = true;
                snapElementRectPreview = {};
                startElementPositions.clear();
                startElementBoundPositions.clear();
                snapVisualRect1 = {0, 0, 0, 0};
                snapVisualRect2 = {0, 0, 0, 0};
                didMove = false;
                moveOlds.clear();
                moveNews.clear();
                if (event->modifiers().testFlag(Qt::ControlModifier) ||
                    event->modifiers().testFlag(Qt::ShiftModifier)) {
                    QList<Element *> newSelected = scene->selectedElements;
                    if (newSelected.contains(clickedElement)) {
                        newSelected.removeOne(clickedElement);
                    } else {
                        newSelected.append(clickedElement);
                    }
                    scene->selectElements(newSelected);
                } else {
                    if (!scene->selectedElements.contains(clickedElement)) {
                        scene->selectElements({clickedElement});
                    }
                }
                for (auto element : scene->selectedElements) {
                    int x = element->x.get(frameInfo);
                    int y = element->y.get(frameInfo);
                    QRect r = element->getBoundingBox(frameInfo);
                    startElementPositions.append(QPoint{x, y});
                    startElementBoundPositions.append(QPoint{r.x(), r.y()});
                    moveOlds.append(element->x.serialize());
                    moveOlds.append(element->y.serialize());
                }
            } else {
                scene->selectElements({});
            }
        }
    }

    if (event->button() == Qt::MiddleButton) {
        if (!dragging && zoom != 1) {
            dragging = true;
            lastDragMousePos = event->position();
            QGuiApplication::setOverrideCursor(
                Qt::CursorShape::ClosedHandCursor);
        }
    }
}

class MoveElementsCommand : public QUndoCommand {
  public:
    MoveElementsCommand(Scene *scene, QList<Element *> elements,
                        QList<QJsonObject> olds, QList<QJsonObject> news)
        : scene(scene), elements(elements), olds(olds), news(news) {
        if (elements.length() == 1) {
            setText("Move " + elements.first()->objectName());
        } else {
            setText("Move " + QString::number(elements.length()) +
                    " element(s)");
        }
    }
    ~MoveElementsCommand() {}

    Scene *scene;
    QList<Element *> elements;
    QList<QJsonObject> olds;
    QList<QJsonObject> news;
    bool didDo{false};

    void undo() override {
        for (int i = 0; i < elements.length(); i++) {
            elements[i]->x.deserialize(olds[i * 2]);
            elements[i]->y.deserialize(olds[i * 2 + 1]);
        }
        scene->selectElements(elements);
        for (auto element : elements) {
            element->_propertyUpdated(&element->x);
        }
    }
    void redo() override {
        if (!didDo) {
            didDo = true;
            return;
        }
        for (int i = 0; i < elements.length(); i++) {
            elements[i]->x.deserialize(news[i * 2]);
            elements[i]->y.deserialize(news[i * 2 + 1]);
        }
        scene->selectElements(elements);
        for (auto element : elements) {
            element->_propertyUpdated(&element->x);
        }
    }
};

class MoveResizeElementCommand : public QUndoCommand {
  public:
    MoveResizeElementCommand(Scene *scene, Element *element, QJsonObject oldX,
                             QJsonObject oldY, QJsonObject oldW,
                             QJsonObject oldH, QJsonObject newX,
                             QJsonObject newY, QJsonObject newW,
                             QJsonObject newH)
        : scene(scene), element(element), oldX(oldX), oldY(oldY), oldW(oldW),
          oldH(oldH), newX(newX), newY(newY), newW(newW), newH(newH) {
        setText("Resize " + element->objectName());
    }
    ~MoveResizeElementCommand() {}

    Scene *scene;
    Element *element;
    QJsonObject oldX;
    QJsonObject oldY;
    QJsonObject oldW;
    QJsonObject oldH;
    QJsonObject newX;
    QJsonObject newY;
    QJsonObject newW;
    QJsonObject newH;
    bool didDo{false};

    void undo() override {
        element->x.deserialize(oldX);
        element->y.deserialize(oldY);
        element->w.deserialize(oldW);
        element->h.deserialize(oldH);
        scene->selectElements(scene->selectedElements); // hack
        element->_propertyUpdated(&element->x);
    }
    void redo() override {
        if (!didDo) {
            didDo = true;
            return;
        }
        element->x.deserialize(newX);
        element->y.deserialize(newY);
        element->w.deserialize(newW);
        element->h.deserialize(newH);
        scene->selectElements(scene->selectedElements); // hack
        element->_propertyUpdated(&element->x);
    }
};

void ImageViewer::mouseReleaseEvent(QMouseEvent *event) {
    if (isPicking && pickType == PickType::Rect) {
        if (event->button() == Qt::LeftButton) {
            stopPicking();
            QToolTip::hideText();
            emit rectPicked(pickId, getPickRect());
            return;
        }
    }

    if (event->button() == Qt::LeftButton) {
        if (isMovingElements || activeResizeMode != -1) {
            // hack to make properties panel update
            scene->selectElements(scene->selectedElements);
        }
        if (isMovingElements) {
            if (didMove) {
                for (auto element : scene->selectedElements) {
                    moveNews.append(element->x.serialize());
                    moveNews.append(element->y.serialize());
                }
                MoveElementsCommand *command = new MoveElementsCommand(
                    scene, scene->selectedElements, moveOlds, moveNews);
                scene->undoStack->push(command);
                moveOlds.clear();
                moveNews.clear();
            }
            isMovingElements = false;
        }
        if (activeResizeMode != -1) {
            MoveResizeElementCommand *command = new MoveResizeElementCommand(
                scene, resizeElement, resizeOldX, resizeOldY, resizeOldW,
                resizeOldH, resizeElement->x.serialize(),
                resizeElement->y.serialize(), resizeElement->w.serialize(),
                resizeElement->h.serialize());
            scene->undoStack->push(command);
            activeResizeMode = -1;
            updateCursor();
        }
    }

    if (event->button() == Qt::MiddleButton) {
        if (dragging) {
            dragging = false;
            QGuiApplication::restoreOverrideCursor();
        }
    }

    if (event->button() == Qt::RightButton) {
        if (moveSpacingMoving) {
            moveSpacingMoving = false;
        }
    }
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event) {
    if (scene->selectedElements.isEmpty())
        return;

    Element *element = scene->selectedElements.first();
    // TODO: not check in imageviewer
    if (dynamic_cast<TextElement *>(element) != nullptr) {
        scene->selectedElements.first()->setEditMode(true);
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

void ImageViewer::beginPicking(const QString &id, const QString &infoText,
                               PickType pickType) {
    isPicking = true;
    startPickPosition = QPoint(-1, -1);
    this->pickType = pickType;
    pickId = id;
    pickText = infoText;
    updateCursor();
    update();
}

void ImageViewer::stopPicking() {
    isPicking = false;
    update();
    updateCursor();
}

void ImageViewer::updateCursor() {
    if (isPicking) {
        setCursor(Qt::CursorShape::CrossCursor);
        return;
    }

    if (activeResizeMode != -1) {
        setCursor(resizeModes[activeResizeMode].cursor);
        return;
    }

    if (hoverResizeMode != -1) {
        setCursor(resizeModes[hoverResizeMode].cursor);
        return;
    }

    setCursor(Qt::CursorShape::ArrowCursor);
}

void ImageViewer::keyPressEvent(QKeyEvent *event) {
    if (textElementEditor) {
        textElementEditor->passKeyEvent(event);
    }
}

void ImageViewer::inputMethodEvent(QInputMethodEvent *event) {
    if (textElementEditor) {
        QKeyEvent *keyEvent =
            new QKeyEvent(QEvent::KeyPress, 0, Qt::KeyboardModifier::NoModifier,
                          event->commitString());
        textElementEditor->passKeyEvent(keyEvent);
        delete keyEvent;
    }
}

bool ImageViewer::event(QEvent *event) {
    if (event->type() == QEvent::ShortcutOverride) {
        if (textElementEditor) {
            event->accept();
            return true;
        }
    }
    return QWidget::event(event);
}
