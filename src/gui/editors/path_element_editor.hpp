#pragma once
#include "animatable/element/path_element.hpp"
#include "editor.hpp"

class PathElementEditor : public Editor {
    Q_OBJECT

    enum HoldType {
        Move,
        ControlPointFrom,
        ControlPointTo,
    };

  public:
    PathElementEditor(NewMainWindow *mainWindow, Scene *scene,
                      PathElement *pathElement, ImageViewer *parent);
    ~PathElementEditor();

    QPoint offset();

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

    void beginHold(const QPoint &pos);

    PathElement *pathElement;
    QList<int> selectedPathPoints;
    int hoveringPointIndex = -1;
    int hoveringControlPointIndex = -1;
    bool hoveringControlPointIsFrom = false;
    bool isClosingPath = false;
    HoldType currentHold;
    QList<QPoint> startHoldPositions;
    QPoint startHoldCursorPosition;

    QCursor closedCursor;
};
