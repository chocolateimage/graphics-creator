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
#include <QStatusBar>
#include <QThread>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidget>

class NewMainWindow;
class RenderWindow;

class FrameTask {
  public:
    ~FrameTask();
    std::vector<ElementRender *> renderElements;
    int width;
    int height;
    int frame;
    double seconds;

    uint64_t id;
    uint32_t *values;

    void render(RenderThread &renderThread);
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
    void taskDone(FrameTask *task);
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

    QAction *playbackAction;
    QAction *deleteAction;

    QAction *renderAction;
    RenderWindow *renderWindow{nullptr};

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
    void taskCompleted(FrameTask *task);
    void elementSelectionChanged(QList<Element *> elements);
    void deleteTriggered();
    void invalidateAndRerender();
    void playbackStateChanged(bool playing);
    void openRenderWindow();

    Scene *scene;

    std::unordered_map<int, SavedFrame> savedFrames;
    QSet<int> renderingFrames;

    uint64_t globalId{0};

    ads::CDockManager *dockManager;
    ads::CDockWidget *propertiesDockWidget;
    QUndoStack *undoStack;

    QList<FramePreviewThread *> previewThreads;
    QList<FrameTask *> openTasks;
    QMutex openTasksMutex;

    TimelineWidget *timeline;

    QLabel *statusText;
};
