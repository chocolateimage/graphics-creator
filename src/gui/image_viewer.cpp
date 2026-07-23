#include "image_viewer.hpp"
#include <QApplication>
#include <QFileDialog>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QMatrix4x4>
#include <QMenu>
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

ImageViewer::ImageViewer(Scene *scene, QWidget *parent)
    : QWidget(parent), scene(scene) {
    setAttribute(Qt::WidgetAttribute::WA_MouseTracking);
    setAttribute(Qt::WidgetAttribute::WA_InputMethodEnabled);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setWindowTitle("Preview");

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
        QSize previewSize{300, 168};
        QSize fullSize = image.size();
        QAction *previewAction = menu.addAction(
            "Preview size (" + QString::number(previewSize.width()) + "x" +
            QString::number(previewSize.height()) + ")");
        QAction *fullAction =
            menu.addAction("Full size (" + QString::number(fullSize.width()) +
                           "x" + QString::number(fullSize.height()) + ")");

        QAction *selectedAction = menu.exec(QCursor::pos());
        if (selectedAction == previewAction) {
            saveFrameAsFile(previewSize);
        } else if (selectedAction == fullAction) {
            saveFrameAsFile(fullSize);
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

void ImageViewer::saveFrameAsFile(QSize size) {
    QString path = QFileDialog::getSaveFileName(
        this, "Save frame", QString(), "PNG image (*.png);;All files (*.*)");
    if (path.isEmpty())
        return;

    image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
        .save(path);
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

    if (scene && !scene->isPlaying()) {
        FrameInfo fi = {scene->currentFrame};
        painter.setPen(QPen(palette().accent(), 2));
        painter.setBrush(Qt::NoBrush);

        for (auto element : scene->selectedElements) {
            QRect boundingBox = element->getBoundingBox(fi);
            QPointF pos = boundingBox.topLeft();
            QPointF bottomRight = boundingBox.bottomRight() + QPoint{1, 1};
            pos = pixelToViewport(pos);
            QPointF size = pixelToViewport(bottomRight) - pos;
            painter.drawRect(pos.x(), pos.y(), size.x() + 1, size.y() + 1);

            if (element->editMode) {
                painter.translate(pos);
                float zoomElement =
                    pixelToViewport({2, 2}).x() - pixelToViewport({1, 1}).x();
                painter.scale(zoomElement, zoomElement);
                textElementEditor->paint(painter);
            }
        }
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

    if (isPicking) {
        QRectF fitRect = fittedRect();
        QPointF pos = current;
        pickPosition = viewportToPixel(current);

        update();
    }

    if (isMovingElements) {
        QPoint newPos = viewportToPixel(current);
        QPoint diff = newPos - startMovePosition;
        startMovePosition = newPos;

        FrameInfo frameInfo{scene->currentFrame};

        for (auto element : scene->selectedElements) {
            element->x.set(element->x.get(frameInfo) + diff.x(), frameInfo);
            element->y.set(element->y.get(frameInfo) + diff.y(), frameInfo);
        }

        update();

        return;
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

    if (event->button() == Qt::LeftButton) {
        if (scene && !scene->isPlaying()) {
            Element *clickedElement{nullptr};
            startMovePosition = viewportToPixel(event->position());

            FrameInfo frameInfo = {scene->currentFrame};
            for (auto element : scene->elements) {
                if (element->getBoundingBox(frameInfo).contains(
                        startMovePosition)) {
                    clickedElement = element;
                    break;
                }
            }

            if (clickedElement) {
                isMovingElements = true;
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

void ImageViewer::mouseReleaseEvent(QMouseEvent *event) {
    if (isPicking && pickType == PickType::Rect) {
        if (event->button() == Qt::LeftButton) {
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
            stopPicking();
            emit rectPicked(pickId, rect);
            return;
        }
    }

    if (event->button() == Qt::LeftButton) {
        if (isMovingElements) {
            isMovingElements = false;
        }
    }

    if (event->button() == Qt::MiddleButton) {
        if (dragging) {
            dragging = false;
            QGuiApplication::restoreOverrideCursor();
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
