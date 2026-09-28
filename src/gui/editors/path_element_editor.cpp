#include "path_element_editor.hpp"
#include "gui/gui.hpp"
#include "gui/image_viewer.hpp"
#include <QApplication>
#include <QPalette>
#include <QWidget>

PathElementEditor::PathElementEditor(NewMainWindow *mainWindow, Scene *scene,
                                     PathElement *pathElement,
                                     ImageViewer *parent)
    : Editor(mainWindow, scene, parent), pathElement(pathElement) {
    QPixmap pix(mainWindow->dataPath + "/assets/pen-cursor-closed.png");
    pix.setDevicePixelRatio(3);
    closedCursor = QCursor(pix, 4, 4);
}

QPoint PathElementEditor::offset() {
    FrameInfo frameInfo{scene->currentFrame};
    return pathElement->getBoundingBox(frameInfo).topLeft() -
           pathElement->getRawBoundingBox(frameInfo).topLeft() +
           QPoint(pathElement->x.get(frameInfo), pathElement->y.get(frameInfo));
}

void PathElementEditor::passKeyEvent(QKeyEvent *keyEvent) {}

void PathElementEditor::paint(QPainter &painter) {
    QPalette palette = imageViewer->palette();

    FrameInfo frameInfo{scene->currentFrame};
    Path path = pathElement->path.get(frameInfo);

    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);

    QPoint offset = this->offset();
    int offsetX = offset.x();
    int offsetY = offset.y();

    for (int i = 0; i < path.points.length(); i++) {
        auto &point = path.points[i];

        if (selectedPathPoints.contains(i)) {
            painter.setPen(Qt::NoBrush);
            painter.setBrush(palette.accent());
        } else {
            painter.setPen(QPen(QColor(128, 128, 128, 200), 2.));
            painter.setBrush(Qt::NoBrush);
        }

        QPointF actualPoint = imageViewer->pixelToViewport(
            QPointF(point.x + offsetX, point.y + offsetY));
        painter.drawRect(actualPoint.x() - 4., actualPoint.y() - 4., 8., 8.);
    }

    painter.setPen(QPen(palette.accent(), 2));
    painter.setBrush(Qt::NoBrush);
    for (auto pointIndex : selectedPathPoints) {
        auto &point = path.points[pointIndex];
        painter.drawLine(imageViewer->pixelToViewport(
                             QPointF(point.x - point.curveX + offsetX,
                                     point.y - point.curveY + offsetY)),
                         imageViewer->pixelToViewport(
                             QPointF(point.x + point.curveX + offsetX,
                                     point.y + point.curveY + offsetY)));
    }
}

bool PathElementEditor::mousePressEvent(const QPoint &pixelPosition,
                                        QMouseEvent *event) {
    FrameInfo frameInfo{scene->currentFrame};
    QPoint offset = this->offset();
    QPoint pos = pixelPosition - offset;
    if (event->buttons().testFlag(Qt::MouseButton::LeftButton)) {
        Path path = pathElement->path.get(frameInfo);

        Qt::KeyboardModifiers keyboardModifiers =
            QApplication::keyboardModifiers();

        if (hoveringPointIndex != -1) {
            if (isClosingPath) {
                path.closed = true;
                pathElement->path.set(path, frameInfo);
                emit closeEditor();
                return true;
            }

            if (keyboardModifiers.testFlag(Qt::ShiftModifier)) {
                if (selectedPathPoints.contains(hoveringPointIndex)) {
                    selectedPathPoints.removeOne(hoveringPointIndex);
                } else {
                    selectedPathPoints.append(hoveringPointIndex);
                }
            } else {
                if (!selectedPathPoints.contains(hoveringPointIndex)) {
                    selectedPathPoints.clear();
                    selectedPathPoints.append(hoveringPointIndex);
                }
            }

            if (keyboardModifiers.testFlag(Qt::AltModifier)) {
                currentHold = HoldType::ControlPoint;
            } else {
                currentHold = HoldType::Move;
            }

            beginHold(pos);

            repaintParent();
            return true;
        }

        path.points.append(PathPoint{pos.x(), pos.y()});
        selectedPathPoints.clear();
        selectedPathPoints.append(path.points.length() - 1);
        pathElement->path.set(path, frameInfo);

        currentHold = HoldType::ControlPoint;
        beginHold(pos);
        return true;
    }

    return false;
}

void PathElementEditor::beginHold(const QPoint &pos) {
    FrameInfo frameInfo{scene->currentFrame};
    Path path = pathElement->path.get(frameInfo);

    startHoldCursorPosition = pos;
    startHoldPositions.clear();

    switch (currentHold) {
    case HoldType::Move: {
        for (int pointIndex : selectedPathPoints) {
            const auto &point = path.points[pointIndex];
            startHoldPositions.append(QPoint(point.x, point.y));
        }
        break;
    }
    case HoldType::ControlPoint: {
        for (int pointIndex : selectedPathPoints) {
            const auto &point = path.points[pointIndex];
            startHoldPositions.append(QPoint(point.curveX, point.curveY));
        }
        break;
    }
    }
}

bool PathElementEditor::mouseMoveEvent(const QPoint &pixelPosition,
                                       QMouseEvent *event) {
    FrameInfo frameInfo{scene->currentFrame};
    QPoint offset = this->offset();
    QPoint pos = pixelPosition - offset;
    QPoint viewportPos = event->position().toPoint();

    Path path = pathElement->path.get(frameInfo);

    if (event->buttons().testFlag(Qt::MouseButton::LeftButton)) {
        int index = 0;

        QPoint moved = pos - startHoldCursorPosition;
        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            if (qAbs(moved.x()) > qAbs(moved.y())) {
                moved = {moved.x(), 0};
            } else {
                moved = {0, moved.y()};
            }
        }

        for (auto pointIndex : selectedPathPoints) {
            QPoint &startPosition = startHoldPositions[index];
            PathPoint &point = path.points[pointIndex];

            if (currentHold == HoldType::Move) {
                point.x = startPosition.x() + moved.x();
                point.y = startPosition.y() + moved.y();
            } else if (currentHold == HoldType::ControlPoint) {
                point.curveX = startPosition.x() + moved.x();
                point.curveY = startPosition.y() + moved.y();
            }

            index++;
        }
        pathElement->path.set(path, frameInfo);
        return true;
    }

    hoveringPointIndex = -1;
    isClosingPath = false;

    int index = 0;
    for (const auto &point : path.points) {
        if ((imageViewer->pixelToViewport(QPoint(point.x, point.y) + offset) -
             viewportPos)
                .manhattanLength() < 12) {

            hoveringPointIndex = index;
            break;
        }
        index++;
    }

    if (hoveringPointIndex != -1) {
        if (!path.closed && selectedPathPoints.length() == 1 &&
            path.points.length() > 1) {
            if ((selectedPathPoints[0] == 0 &&
                 hoveringPointIndex == path.points.length() - 1) ||
                (selectedPathPoints[0] == path.points.length() - 1 &&
                 hoveringPointIndex == 0)) {
                isClosingPath = true;
            }
        }

        if (isClosingPath) {
            cursor = closedCursor;
        } else {
            cursor = Qt::ArrowCursor;
        }
    } else {
        cursor = Qt::BlankCursor;
    }
    emit cursorChanged();

    return false;
}

bool PathElementEditor::mouseReleaseEvent(const QPoint &pixelPosition,
                                          QMouseEvent *event) {
    return false;
}

PathElementEditor::~PathElementEditor() {}
