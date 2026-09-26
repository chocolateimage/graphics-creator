#include "editor.hpp"
#include "gui/image_viewer.hpp"
#include <QWidget>

Editor::Editor(NewMainWindow *mainWindow, Scene *scene, ImageViewer *parent)
    : QObject(parent), mainWindow(mainWindow), scene(scene),
      imageViewer(parent) {}

void Editor::repaintParent() { imageViewer->update(); }

Editor::~Editor() {}
