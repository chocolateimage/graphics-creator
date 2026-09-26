#pragma once
#include "scene.hpp"
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

    void repaintParent();

    virtual void passKeyEvent(QKeyEvent *keyEvent) = 0;
    virtual void paint(QPainter &painter) = 0;
    virtual bool shouldTransformPainter() = 0;
};
