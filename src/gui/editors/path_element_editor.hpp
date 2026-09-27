#pragma once
#include "animatable/element/path_element.hpp"
#include "editor.hpp"

class PathElementEditor : public Editor {
    Q_OBJECT
  public:
    PathElementEditor(NewMainWindow *mainWindow, Scene *scene,
                      PathElement *pathElement, ImageViewer *parent);
    ~PathElementEditor();

    void passKeyEvent(QKeyEvent *keyEvent) override;
    void paint(QPainter &painter) override;
    bool shouldTransformPainter() override { return false; }
    bool shouldShowElementBorder() override { return false; };

    bool mousePressEvent(const QPoint &pixelPosition,
                         QMouseEvent *event) override;
    bool mouseMoveEvent(const QPoint &pixelPosition,
                        QMouseEvent *event) override;
    bool mouseReleaseEvent(const QPoint &pixelPosition,
                           QMouseEvent *event) override;

    PathElement *pathElement;
    QList<int> selectedPathPoints;
};
