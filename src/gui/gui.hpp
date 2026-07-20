#pragma once
#include "image_viewer.hpp"
#include "lua.hpp"
#include "scene.hpp"
#include "timeline.hpp"
#include <DockManager.h>
#include <QChronoTimer>
#include <QComboBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMutex>
#include <QPushButton>
#include <QStackedWidget>
#include <QThread>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidget>

class NewMainWindow;

class FramePreviewTask {
  public:
    std::vector<ElementRender *> renderElements;
    int width;
    int height;
    int frame;
    double seconds;

    uint64_t id;
    uint32_t *values;
};

struct SavedFrame {
    uint32_t *values;
    uint64_t id;
};

class FramePreviewThread : public QThread {
    Q_OBJECT
  public:
    explicit FramePreviewThread(QObject *parent = nullptr) : QThread(parent) {}
    NewMainWindow *window{nullptr};
    std::atomic<bool> stayAlive{true};

  protected:
    void run() override;
  signals:
    void taskDone(FramePreviewTask *task);
};

class NewMainWindow : public QMainWindow {
  public:
    NewMainWindow();
    ~NewMainWindow();

    QAction *controlSelect;
    QAction *controlRectangle;
    QAction *controlEllipse;
    QAction *controlPolygon;
    QAction *controlText;
    QAction *controlLua;

    QAction *deleteAction;

    ImageViewer *scenePreviewWidget;

    void loadDefaultFont();
    void createThread();
    void controlsUpdated();
    void sceneRectPicked(QString id, QRect rect);
    void updatePreview();
    void rerender();
    void elementAdded(Element *element, int index);
    void elementUpdated(Element *element);
    void elementOrderChanged();
    void frameChanged(int frame);
    bool createTask(int frame);
    void invalidateFrame(int frame);
    void taskCompleted(FramePreviewTask *task);
    void elementSelectionChanged(QList<Element *> elements);
    void deleteTriggered();
    void invalidateAndRerender();

    Scene *scene;

    std::unordered_map<int, SavedFrame> savedFrames;
    QSet<int> renderingFrames;

    uint64_t globalId;

    ads::CDockManager *dockManager;
    QUndoStack *undoStack;

    QList<FramePreviewThread *> previewThreads;
    QList<FramePreviewTask *> openTasks;
    QMutex openTasksMutex;

    TimelineWidget *timeline;
};
