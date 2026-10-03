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

void PathElementEditor::passKeyEvent(QKeyEvent *keyEvent) {
    FrameInfo frameInfo{scene->currentFrame};
    if (keyEvent->matches(QKeySequence::Delete)) {
        Path path = pathElement->path.get(frameInfo);
        std::sort(selectedPathPoints.begin(), selectedPathPoints.end(),
                  [](int a, int b) { return a > b; });
        for (auto pointIndex : selectedPathPoints) {
            path.points.removeAt(pointIndex);
        }
        selectedPathPoints.clear();
        pathElement->path.set(path, frameInfo);
        repaintParent();
    } else if (keyEvent->matches(QKeySequence::SelectAll)) {
        Path path = pathElement->path.get(frameInfo);
        selectedPathPoints.clear();
        for (int i = 0; i < path.points.length(); i++) {
            selectedPathPoints.append(i);
        }
        repaintParent();
    }
}

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

    painter.setBrush(Qt::NoBrush);
    for (auto pointIndex : selectedPathPoints) {
        auto &point = path.points[pointIndex];
        if (point.curveX == 0 && point.curveY == 0)
            continue;

        QPoint fromPoint =
            imageViewer
                ->pixelToViewport(QPointF(point.x - point.curveX + offsetX,
                                          point.y - point.curveY + offsetY))
                .toPoint();
        QPoint toPoint =
            imageViewer
                ->pixelToViewport(QPointF(point.x + point.curveX + offsetX,
                                          point.y + point.curveY + offsetY))
                .toPoint();

        painter.setPen(QPen(palette.accent(), 2));
        painter.drawLine(fromPoint, toPoint);

        painter.setPen(QPen(palette.accent(), 6));
        painter.drawPoint(fromPoint);
        painter.drawPoint(toPoint);
    }
}

bool PathElementEditor::mousePressEvent(const QPoint &pixelPosition,
                                        QMouseEvent *event) {
    FrameInfo frameInfo{scene->currentFrame};
    QPoint offset = this->offset();
    QPoint pos = pixelPosition - offset;
    if (event->button() == Qt::MouseButton::LeftButton) {
        Path path = pathElement->path.get(frameInfo);

        Qt::KeyboardModifiers keyboardModifiers =
            QApplication::keyboardModifiers();

        if (hoveringControlPointIndex != -1) {
            selectedPathPoints.clear();
            selectedPathPoints.append(hoveringControlPointIndex);
            currentHold = hoveringControlPointIsFrom
                              ? HoldType::ControlPointFrom
                              : HoldType::ControlPointTo;

            beginHold(pos);

            repaintParent();
            return true;
        }

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
                currentHold = HoldType::ControlPointTo;
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

        currentHold = HoldType::ControlPointTo;
        beginHold(pos);
        return true;
    } else if (event->button() == Qt::MouseButton::RightButton) {
        emit closeEditor();
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
    case HoldType::ControlPointFrom:
    case HoldType::ControlPointTo: {
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
            } else if (currentHold == HoldType::ControlPointFrom) {
                point.curveX = startPosition.x() - moved.x();
                point.curveY = startPosition.y() - moved.y();
            } else if (currentHold == HoldType::ControlPointTo) {
                point.curveX = startPosition.x() + moved.x();
                point.curveY = startPosition.y() + moved.y();
            }

            index++;
        }
        pathElement->path.set(path, frameInfo);
        return true;
    }

    hoveringPointIndex = -1;
    hoveringControlPointIndex = -1;
    isClosingPath = false;

    int index = 0;
    for (const auto &point : path.points) {
        if ((imageViewer->pixelToViewport(QPoint(point.x, point.y) + offset) -
             viewportPos)
                .manhattanLength() < 12) {

            hoveringPointIndex = index;
            break;
        }

        if (point.curveX == 0 && point.curveY == 0) {
            index++;
            continue;
        }

        QPoint fromPoint =
            imageViewer
                ->pixelToViewport(
                    QPointF(point.x - point.curveX, point.y - point.curveY) +
                    offset)
                .toPoint();
        QPoint toPoint = imageViewer
                             ->pixelToViewport(QPointF(point.x + point.curveX,
                                                       point.y + point.curveY) +
                                               offset)
                             .toPoint();
        bool hoveringFromPoint =
            (fromPoint - viewportPos).manhattanLength() < 12;
        bool hoveringToPoint = (toPoint - viewportPos).manhattanLength() < 12;

        if (hoveringFromPoint || hoveringToPoint) {
            hoveringControlPointIndex = index;
            hoveringControlPointIsFrom = hoveringFromPoint;
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
    }

    if (hoveringPointIndex != -1 || hoveringControlPointIndex != -1) {
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
