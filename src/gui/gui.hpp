#pragma once
#include "image_viewer.hpp"
#include "lua.hpp"
#include "scene.hpp"
#include <DockManager.h>
#include <QChronoTimer>
#include <QComboBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidget>

class NewMainWindow : public QMainWindow {
  public:
    NewMainWindow();
    ~NewMainWindow();

    QAction *controlSelect;
    QAction *controlRectangle;
    QAction *controlEllipse;
    QAction *controlPolygon;
    QAction *controlLua;

    ImageViewer *scenePreviewWidget;

    void controlsUpdated();
    void sceneRectPicked(QString id, QRect rect);
    void renderTest();
    void rerender();
    void elementUpdated(Element *element);
    void frameChanged(int frame);

    Scene *scene;

    ads::CDockManager *dockManager;
    QUndoStack *undoStack;
};
