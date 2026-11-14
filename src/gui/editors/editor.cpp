#include "editor.hpp"
#include "gui/gui.hpp"
#include "gui/image_viewer.hpp"
#include <QApplication>
#include <QWidget>

Editor::Editor(NewMainWindow *mainWindow, Scene *scene, ImageViewer *parent)
    : QObject(parent), mainWindow(mainWindow), scene(scene),
      imageViewer(parent), cursor(Qt::BlankCursor) {}

void Editor::repaintParent() { imageViewer->update(); }

void Editor::setStatusTip(const QString &message) {
    imageViewer->setStatusTip(message);
    QStatusTipEvent event(message);
    QApplication::sendEvent(mainWindow, &event);
}

Editor::~Editor() {}
