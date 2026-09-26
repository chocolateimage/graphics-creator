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
    bool shouldTransformPainter() override { return false; };

    PathElement *pathElement;
};
