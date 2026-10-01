#pragma once
#include "scene.hpp"
#include <QCursor>
#include <QKeyEvent>
#include <QObject>
#include <QPainter>

class NewMainWindow;
class ImageViewer;

class Editor : public QObject {
    Q_OBJECT
  public:
    Editor(NewMainWindow *mainWindow, Scene *scene, ImageViewer *parent);
    virtual ~Editor();

    NewMainWindow *mainWindow;
    Scene *scene;
    ImageViewer *imageViewer;
    QCursor cursor;

    void repaintParent();

    virtual void passKeyEvent(QKeyEvent *keyEvent) {}
    virtual void passKeyReleaseEvent(QKeyEvent *keyEvent) {}
    virtual void paint(QPainter &painter) {}
    virtual bool mousePressEvent(const QPoint &pixelPosition,
                                 QMouseEvent *event) {
        return false;
    }
    virtual bool mouseMoveEvent(const QPoint &pixelPosition,
                                QMouseEvent *event) {
        return false;
    }
    virtual bool mouseReleaseEvent(const QPoint &pixelPosition,
                                   QMouseEvent *event) {
        return false;
    }

    virtual bool shouldTransformPainter() = 0;
    virtual bool shouldShowElementBorder() { return true; };

  signals:
    void closeEditor();
    void cursorChanged();
};
