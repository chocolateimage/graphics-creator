#include "path_element_editor.hpp"
#include "gui/image_viewer.hpp"
#include <QPalette>
#include <QWidget>

PathElementEditor::PathElementEditor(NewMainWindow *mainWindow, Scene *scene,
                                     PathElement *pathElement,
                                     ImageViewer *parent)
    : Editor(mainWindow, scene, parent), pathElement(pathElement) {}

void PathElementEditor::passKeyEvent(QKeyEvent *keyEvent) {}

void PathElementEditor::paint(QPainter &painter) {
    QPalette palette = imageViewer->palette();
    float scale = painter.transform().m11();

    Path path = pathElement->path.get({scene->currentFrame});

    painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);

    for (int i = 0; i < path.points.length(); i++) {
        auto &point = path.points[i];

        // if (selectedPathPoints.contains(i)) {
        //     painter.setPen(Qt::NoBrush);
        //     painter.setBrush(palette.accent());
        // } else {
        painter.setPen(QPen(QColor(128, 128, 128, 200), 2. / scale));
        painter.setBrush(Qt::NoBrush);
        // }

        QPointF actualPoint =
            imageViewer->pixelToViewport({(qreal)point.x, (qreal)point.y});
        painter.drawRect(actualPoint.x() - 4. / scale,
                         actualPoint.y() - 4. / scale, 8. / scale, 8. / scale);
    }

    painter.setPen(QPen(palette.accent(), 2));
    painter.setBrush(Qt::NoBrush);
    // for (auto pointIndex : selectedPathPoints) {
    //     auto &point = path.points[pointIndex];
    //     painter.drawLine(pixelToViewport(QPointF(point.x - point.curveX,
    //                                              point.y - point.curveY)),
    //                      pixelToViewport(QPointF(point.x + point.curveX,
    //                                              point.y + point.curveY)));
    // }
}

PathElementEditor::~PathElementEditor() {}
