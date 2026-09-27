#include "path_element_editor.hpp"
#include "gui/image_viewer.hpp"
#include <QPalette>
#include <QWidget>

PathElementEditor::PathElementEditor(NewMainWindow *mainWindow, Scene *scene,
                                     PathElement *pathElement,
                                     ImageViewer *parent)
    : Editor(mainWindow, scene, parent), pathElement(pathElement) {}

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
    QPoint pos = pixelPosition - offset();
    if (event->buttons().testFlag(Qt::MouseButton::LeftButton)) {
        Path path = pathElement->path.get(frameInfo);

        if (selectedPathPoints.length() == 1 &&
            (selectedPathPoints[0] == 0 ||
             selectedPathPoints[0] == path.points.length() - 1)) {
            bool shouldClose = false;
            const auto &point = path.points[selectedPathPoints[0] == 0
                                                ? path.points.length() - 1
                                                : 0];
            if (sqrt(pow((point.x - pos.x()), 2) +
                     pow((point.y - pos.y()), 2)) < 30) {
                shouldClose = true;
            }

            if (shouldClose) {
                path.closed = true;
                pathElement->path.set(path, frameInfo);
                emit closeEditor();
                return true;
            }
        }

        path.points.append(PathPoint{pos.x(), pos.y()});
        selectedPathPoints.clear();
        selectedPathPoints.append(path.points.length() - 1);
        pathElement->path.set(path, frameInfo);
        return true;
    }

    return false;
}

bool PathElementEditor::mouseMoveEvent(const QPoint &pixelPosition,
                                       QMouseEvent *event) {
    QPoint pos = pixelPosition - offset();
    if (event->buttons().testFlag(Qt::MouseButton::LeftButton)) {
        Path path = pathElement->path.get({scene->currentFrame});
        for (auto pointIndex : selectedPathPoints) {
            PathPoint &point = path.points[pointIndex];
            point.curveX = pos.x() - point.x;
            point.curveY = pos.y() - point.y;
        }
        pathElement->path.set(path, {scene->currentFrame});
        return true;
    }

    return false;
}

bool PathElementEditor::mouseReleaseEvent(const QPoint &pixelPosition,
                                          QMouseEvent *event) {
    return false;
}

PathElementEditor::~PathElementEditor() {}
