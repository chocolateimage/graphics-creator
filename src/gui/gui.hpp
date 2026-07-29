#pragma once
#include "image_viewer.hpp"
#include "scene.hpp"
#include "timeline.hpp"
#include <DockManager.h>
#include <QChronoTimer>
#include <QComboBox>
#include <QDialog>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMutex>
#include <QNetworkAccessManager>
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

class VideoSettingsDialog : public QDialog {
    Q_OBJECT
  public:
    explicit VideoSettingsDialog(Scene *scene, QWidget *parent = nullptr);
    Scene *scene;

  private slots:
    void save();
    void updateDurationFrames();

  private:
    QSpinBox *width;
    QSpinBox *height;
    QSpinBox *frameRate;
    QDoubleSpinBox *duration;
    QLabel *durationFramesLabel;
};

class NewMainWindow : public QMainWindow {
  public:
    NewMainWindow();
    ~NewMainWindow();

    QToolBar *toolBar;
    QMenu *editMenu;
    QMenu *videoMenu;
    QMenu *viewMenu;
    QAction *controlSelect;
    QAction *controlRectangle;
    QAction *controlEllipse;
    QAction *controlPolygon;
    QAction *controlText;
    QAction *controlLua;

    QStatusBar *statusBar;

    QAction *saveAction;
    QAction *saveAsAction;

    QAction *playbackAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;

    QAction *renderAction;
    RenderWindow *renderWindow{nullptr};

    ImageViewer *scenePreviewWidget;

    void loadDefaultFont();
    void createThread();
    void controlsUpdated();
    void sceneRectPicked(QString id, QRect rect);
    void updatePreview();
    void rerender(bool onlyCurrentFrame);
    void elementAdded(Element *element, int index);
    void elementUpdated(Element *element);
    void elementOrderChanged();
    void frameChanged(int frame);
    bool createTask(int frame);
    void taskCompleted(FrameTask *task);
    void elementSelectionChanged(QList<Element *> elements);
    void deleteTriggered();
    void invalidateAndRerender();
    void invalidateAndRerender_afterDelay();
    void playbackStateChanged(bool playing);
    void openRenderWindow();
    bool saveSlot();
    bool saveAsSlot();
    bool save();
    void openSlot();
    void openVideoSettings();
    bool askSaveConfirmation();
    void addElementUndoable(Element *element);
    void copySlot();
    void pasteSlot();
    void clipboardContentsChanged();
    void previousFrameSlot();
    void nextFrameSlot();
    void invalidateFrame(int frame);
    void saveLayout();
    void showWelcome(bool show);
    void newProject(int width, int height, double frameRate,
                    int durationFrames);
    void newSlot();
    void checkForUpdates();
    void notLatestVersion(QJsonObject obj);

    QNetworkAccessManager *networkAccessManager;

    QString dataPath;

    QJsonDocument saveInto();
    void loadFile(const QString &filePath);
    bool loadFrom(const QJsonDocument &document);
    QString openFilePath;
    void setOpenFilePath(const QString &newPath);
    Element *loadElementFromJson(const QJsonObject &obj);

    Scene *scene;

    std::unordered_map<int, SavedFrame> savedFrames;
    QSet<int> renderingFrames;
    QList<FrameTask *> inProgressTasks;

    uint64_t globalId{0};

    QStackedWidget *mainStackWidget;
    ads::CDockManager *dockManager;
    ads::CDockWidget *propertiesDockWidget;
    QUndoStack *undoStack;

    QList<FramePreviewThread *> previewThreads;
    QList<FrameTask *> openTasks;
    QMutex openTasksMutex;

    TimelineWidget *timeline;

    QLabel *statusText;

    QTimer lastRenderDelayTimer;

  protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};
