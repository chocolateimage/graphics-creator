#include "gui.hpp"
#include "animatable/effect/effect_list.hpp"
#include "animatable/element/ellipse_element.hpp"
#include "animatable/element/group_element.hpp"
#include "animatable/element/image_element.hpp"
#include "animatable/element/rectangle_element.hpp"
#include "animatable/element/text_element.hpp"
#include "animatable/element/video_element.hpp"
#include "effects_window.hpp"
#include "property_window.hpp"
#include "render.hpp"
#include "render_window.hpp"
#include "timeline/timeline_content.hpp"
#include "variant.hpp"
#include "welcome_popup.hpp"
#include "welcome_screen.hpp"
#include <DockAreaWidget.h>
#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
#include <KIconTheme>
#include <KMessageBox>
#include <KStyleManager>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QMimeDatabase>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressDialog>
#include <QRandomGenerator>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QToolBar>
#include <fontconfig/fontconfig.h>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

const QString VERSION = "0.4.0";

const QString ELEMENT_COPY_MIME_TYPE =
    "application/x-graphicscreator-element-copy";

void FramePreviewThread::run() {
    RenderThread renderThread;
    renderThread.init();

    while (stayAlive) {
        QThread::msleep(3);
        FrameTask *task;
        {
            QMutexLocker lock(&window->openTasksMutex);
            if (window->openTasks.isEmpty()) {
                continue;
            }

            task = window->openTasks.front();
            window->openTasks.pop_front();
        }

        QElapsedTimer renderTime;
        renderTime.start();

        task->render(renderThread);

        // qInfo() << "Render time:"
        //         << qPrintable(QString("%1").arg(
        //                renderTime.nsecsElapsed() / 1000000., 0, 'f', 1))
        //         << "ms";

        emit taskDone(task);

        renderThread.garbageCollect();
    }

    renderThread.close();
}

VideoSettingsDialog::VideoSettingsDialog(Scene *scene, QWidget *parent)
    : QDialog(parent), scene(scene) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Video Settings");
    auto parentLay = new QVBoxLayout(this);
    auto lay = new QFormLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    parentLay->addLayout(lay);

    width = new QSpinBox(this);
    width->setRange(1, 999999);
    width->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    height = new QSpinBox(this);
    height->setRange(1, 999999);
    height->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    frameRate = new QDoubleSpinBox(this);
    frameRate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frameRate->setRange(.1, 999999);

    duration = new QDoubleSpinBox(this);
    duration->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    duration->setRange(0, 999999);
    duration->setSuffix(" seconds");

    durationFramesLabel = new QLabel(this);

    width->setValue(scene->width);
    height->setValue(scene->height);
    frameRate->setValue(scene->frameRate);
    duration->setValue((double)scene->durationFrames / scene->frameRate);
    connect(frameRate, &QDoubleSpinBox::valueChanged, this,
            &VideoSettingsDialog::updateDurationFrames);
    connect(duration, &QDoubleSpinBox::valueChanged, this,
            &VideoSettingsDialog::updateDurationFrames);
    updateDurationFrames();

    lay->addRow("Width", width);
    lay->addRow("Height", height);
    lay->addRow("Frame Rate", frameRate);
    lay->addRow("Duration", duration);
    lay->addWidget(durationFramesLabel);

    setMinimumWidth(400);

    parentLay->addStretch();

    auto buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    parentLay->addWidget(buttonBox);

    connect(this, &VideoSettingsDialog::accepted, this,
            &VideoSettingsDialog::save);
}

void VideoSettingsDialog::updateDurationFrames() {
    durationFramesLabel->setText(
        "= " + QString::number((int)(duration->value() * frameRate->value())) +
        " frames");
}

void VideoSettingsDialog::save() {
    QList<Element *> lastElements;
    for (auto element : scene->elements) {
        if (element->startFrame + element->durationFrames ==
            scene->durationFrames) {
            lastElements.append(element);
        }
    }
    scene->frameRate = frameRate->value();
    scene->width = width->value();
    scene->height = height->value();
    scene->durationFrames = (int)(duration->value() * frameRate->value());
    for (auto element : lastElements) {
        if (element->startFrame >= scene->durationFrames) {
            element->startFrame = scene->durationFrames - 1;
        }
        element->durationFrames =
            std::max(1, scene->durationFrames - element->startFrame);
    }
    emit scene->sceneInfoChanged();
}

class AddElementCommand : public QUndoCommand {
  public:
    AddElementCommand(Scene *scene, Element *element)
        : scene(scene), element(element) {
        setText(element->objectName());
    }
    ~AddElementCommand() {
        if (undid) {
            delete element;
        }
    }

    Scene *scene;
    Element *element;
    QList<Element *> oldSelected;
    bool undid{false};

    void undo() override {
        undid = true;
        scene->selectElements(oldSelected);
        scene->removeElement(element);
    }
    void redo() override {
        undid = false;
        oldSelected = scene->selectedElements;
        scene->insertElement(element, 0);
        scene->selectElements({element});
    }
};

class RemoveElementsCommand : public QUndoCommand {
  public:
    RemoveElementsCommand(Scene *scene, QList<Element *> elements)
        : scene(scene), elements(elements) {
        setText("Delete " + QString::number(elements.length()) + " element(s)");
    }
    ~RemoveElementsCommand() {
        if (undid)
            return;
        for (auto element : elements) {
            delete element;
        }
    }

    Scene *scene;
    QList<Element *> elements;
    QList<Element *> oldSelected;
    QList<Element *> oldOrder;
    bool undid{false};

    void undo() override {
        undid = true;
        for (auto element : elements) {
            scene->addElement(element);
        }
        scene->elements = oldOrder;
        emit scene->elementOrderChanged();
        scene->selectElements(oldSelected);
    }
    void redo() override {
        undid = false;
        oldSelected = scene->selectedElements;
        oldOrder = scene->elements;
        for (auto element : elements) {
            scene->removeElement(element);
        }
        scene->selectElements({});
    }
};

NewMainWindow::NewMainWindow() : QMainWindow() {
    QStringList dataPaths = {
        QApplication::applicationDirPath() + "/data",
        QApplication::applicationDirPath() + "/../share/graphics-creator/data",
    };

    for (auto path : dataPaths) {
        if (QDir(path).exists()) {
            dataPath = QDir(path).absolutePath();
            break;
        }
    }

    if (dataPath.isEmpty()) {
        qCritical() << "No data path found. Tried:" << dataPaths;
        KMessageBox::error(
            nullptr, "No data folder found.\n\nPlease try reinstalling the "
                     "program. If this "
                     "problem persists after reinstalling, report an issue.");
        exit(1);
    }

    networkAccessManager = new QNetworkAccessManager(this);
    scene = new Scene();
    scene->undoStack = new QUndoStack(this);
    scene->undoStack->setUndoLimit(50);
    scene->width = 1280;
    scene->height = 720;
    scene->frameRate = 30;
    scene->durationFrames = scene->frameRate * 5;
    scene->canContinuePlayback = [this]() {
        bool realtime = !renderingFrames.contains(scene->currentFrame);
        if (realtime) {
            statusText->setEnabled(false);
            statusText->setStyleSheet("");
            statusText->setText("Realtime playback");
        } else {
            statusText->setEnabled(true);
            statusText->setStyleSheet("color: orange;");
            statusText->setText("Not realtime playback");
        }
        return realtime;
    };

    std::sort(effectList.begin(), effectList.end(),
              [](const EffectInfo &a, const EffectInfo &b) {
                  return a.sortString() < b.sortString();
              });

    lastRenderDelayTimer.setSingleShot(true);
    lastRenderDelayTimer.setInterval(100);
    connect(&lastRenderDelayTimer, &QTimer::timeout, this,
            &NewMainWindow::invalidateAndRerender_afterDelay);

    connect(scene, &Scene::elementAdded, this, &NewMainWindow::elementAdded);
    connect(scene, &Scene::elementUpdated, this,
            &NewMainWindow::elementUpdated);
    connect(scene, &Scene::elementRemoved, this,
            &NewMainWindow::elementUpdated); // hack
    connect(scene, &Scene::elementOrderChanged, this,
            &NewMainWindow::elementOrderChanged);
    connect(scene, &Scene::sceneInfoChanged, this,
            &NewMainWindow::invalidateAndRerender);
    connect(scene, &Scene::frameChanged, this, &NewMainWindow::frameChanged);
    connect(scene, &Scene::elementSelectionChanged, this,
            &NewMainWindow::elementSelectionChanged);
    connect(scene, &Scene::playbackStateChanged, this,
            &NewMainWindow::playbackStateChanged);

    loadDefaultFont();

    this->resize(1200, 700);

    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize,
                                     true);
    ads::CDockManager::setAutoHideConfigFlags(
        ads::CDockManager::DefaultAutoHideConfig);

    statusBar = new QStatusBar(this);
    statusText = new QLabel(statusBar);
    statusBar->addPermanentWidget(statusText);
    setStatusBar(statusBar);

    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *fileMenu = menuBar->addMenu("File");

    QAction *newAction = fileMenu->addAction("New");
    newAction->setIcon(QIcon::fromTheme("document-new"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &NewMainWindow::newSlot);

    QAction *openAction = fileMenu->addAction("Open…");
    openAction->setIcon(QIcon::fromTheme("document-open-data"));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &NewMainWindow::openSlot);

    saveAction = fileMenu->addAction("Save");
    saveAction->setIcon(QIcon::fromTheme("document-save"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &NewMainWindow::saveSlot);

    saveAsAction = fileMenu->addAction("Save as…");
    saveAsAction->setIcon(QIcon::fromTheme("document-save"));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this,
            &NewMainWindow::saveAsSlot);

    fileMenu->addSeparator();
    QAction *insertSeparator = fileMenu->addSeparator();

    QAction *quitAction = fileMenu->addAction("Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(quitAction, &QAction::triggered, this, &NewMainWindow::close);

    editMenu = menuBar->addMenu("Edit");
    QAction *undoAction = scene->undoStack->createUndoAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAction);
    QAction *redoAction = scene->undoStack->createRedoAction(this);
    redoAction->setShortcuts(QKeySequence::Redo);
    editMenu->addAction(redoAction);

    editMenu->addSeparator();
    copyAction = editMenu->addAction("Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setIcon(QIcon::fromTheme("edit-copy"));
    connect(copyAction, &QAction::triggered, this, &NewMainWindow::copySlot);

    pasteAction = editMenu->addAction("Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setIcon(QIcon::fromTheme("edit-paste"));
    connect(pasteAction, &QAction::triggered, this, &NewMainWindow::pasteSlot);

    duplicateAction = editMenu->addAction("Duplicate");
    duplicateAction->setShortcut(QKeySequence("Ctrl+D"));
    duplicateAction->setIcon(QIcon::fromTheme("edit-duplicate"));
    connect(duplicateAction, &QAction::triggered, this,
            &NewMainWindow::duplicateSlot);

    deleteAction = editMenu->addAction("Delete");
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setIcon(QIcon::fromTheme("delete"));
    connect(deleteAction, &QAction::triggered, this,
            &NewMainWindow::deleteTriggered);

    editMenu->addSeparator();
    groupAction = editMenu->addAction("Group");
    groupAction->setShortcut(QKeySequence("Ctrl+G"));
    groupAction->setIcon(QIcon::fromTheme("object-group"));
    connect(groupAction, &QAction::triggered, this, &NewMainWindow::groupSlot);

    ungroupAction = editMenu->addAction("Ungroup");
    ungroupAction->setShortcut(QKeySequence("Ctrl+Shift+G"));
    ungroupAction->setIcon(QIcon::fromTheme("object-ungroup"));
    connect(ungroupAction, &QAction::triggered, this,
            &NewMainWindow::ungroupSlot);

    videoMenu = menuBar->addMenu("Video");

    QAction *goToStartAction = videoMenu->addAction("Go to start");
    goToStartAction->setIcon(QIcon::fromTheme("media-skip-backward"));

    playbackAction = videoMenu->addAction("");
    playbackAction->setShortcut(QKeySequence(" "));

    QAction *previousFrameAction = videoMenu->addAction("Previous frame");
    previousFrameAction->setShortcuts(
        {QKeySequence("Ctrl+Left"), QKeySequence("PgUp")});
    connect(previousFrameAction, &QAction::triggered, this,
            &NewMainWindow::previousFrameSlot);
    QAction *nextFrameAction = videoMenu->addAction("Next frame");
    nextFrameAction->setShortcuts(
        {QKeySequence("Ctrl+Right"), QKeySequence("PgDown")});
    connect(nextFrameAction, &QAction::triggered, this,
            &NewMainWindow::nextFrameSlot);

    videoMenu->addSeparator();

    QAction *previousKeyframeAction = videoMenu->addAction("Previous keyframe");
    previousKeyframeAction->setShortcuts({QKeySequence("J")});
    connect(previousKeyframeAction, &QAction::triggered, this,
            &NewMainWindow::previousKeyframeSlot);
    QAction *nextKeyframeAction = videoMenu->addAction("Next keyframe");
    nextKeyframeAction->setShortcuts({QKeySequence("K")});
    connect(nextKeyframeAction, &QAction::triggered, this,
            &NewMainWindow::nextKeyframeSlot);

    videoMenu->addSeparator();

    QAction *videoSettingsAction = videoMenu->addAction("Settings…");
    videoSettingsAction->setIcon(QIcon::fromTheme("settings-configure"));
    connect(videoSettingsAction, &QAction::triggered, this,
            &NewMainWindow::openVideoSettings);

    viewMenu = menuBar->addMenu("View");
    viewMenu->setToolTipsVisible(true);

    KActionMenu *colorMenu =
        KColorSchemeMenu::createMenu(KColorSchemeManager::instance());
    viewMenu->addMenu(colorMenu->menu());
    viewMenu->addSeparator();

    setMenuBar(menuBar);

    toolBar = addToolBar("Controls");
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    QActionGroup *controlsGroup = new QActionGroup(toolBar);
    controlSelect = toolBar->addAction(QIcon::fromTheme("select"), "Select");
    controlSelect->setShortcut(QKeySequence("V"));
    toolBar->addSeparator();
    controlRectangle =
        toolBar->addAction(QIcon::fromTheme("draw-rectangle"), "Rectangle");
    controlRectangle->setShortcut(QKeySequence("R"));
    toolBar->widgetForAction(controlRectangle)->installEventFilter(this);
    controlEllipse =
        toolBar->addAction(QIcon::fromTheme("draw-circle"), "Ellipse");
    controlEllipse->setShortcut(QKeySequence("E"));
    controlText = toolBar->addAction(QIcon::fromTheme("draw-text"), "Text");
    controlText->setShortcut(QKeySequence("T"));
    toolBar->addSeparator();
    controlImport = toolBar->addAction(QIcon::fromTheme("download"), "Import…");
    controlImport->setToolTip("Add media files to scene");
    controlImport->setShortcut(QKeySequence("Ctrl+I"));

    editMenu->addSeparator();

    editMenu->addAction(controlSelect);
    editMenu->addAction(controlRectangle);
    editMenu->addAction(controlEllipse);
    editMenu->addAction(controlText);

    fileMenu->insertAction(insertSeparator, controlImport);

    controlSelect->setActionGroup(controlsGroup);
    controlRectangle->setActionGroup(controlsGroup);
    controlEllipse->setActionGroup(controlsGroup);
    controlText->setActionGroup(controlsGroup);

    controlSelect->setCheckable(true);
    controlRectangle->setCheckable(true);
    controlEllipse->setCheckable(true);
    controlText->setCheckable(true);

    controlSelect->setChecked(true);

    connect(controlSelect, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlRectangle, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlEllipse, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlText, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlImport, &QAction::triggered, this,
            &NewMainWindow::importClicked);

    QWidget *spacing = new QWidget(toolBar);
    spacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->addWidget(spacing);

    renderAction =
        toolBar->addAction(QIcon::fromTheme("media-record"), "Render…");
    connect(renderAction, &QAction::triggered, this,
            &NewMainWindow::openRenderWindow);

    mainStackWidget = new QStackedWidget(this);
    setCentralWidget(mainStackWidget);

    WelcomeScreenWidget *welcomeScreen =
        new WelcomeScreenWidget(this, mainStackWidget);
    connect(welcomeScreen, &WelcomeScreenWidget::newProjectClicked, this,
            &NewMainWindow::newSlot);
    connect(welcomeScreen, &WelcomeScreenWidget::openClicked, this,
            &NewMainWindow::welcomeOpenClicked);
    mainStackWidget->addWidget(welcomeScreen);

    dockManager = new ads::CDockManager(mainStackWidget);
    mainStackWidget->addWidget(dockManager);

    ads::CDockWidget *sceneDockWidget = dockManager->createDockWidget("Scene");
    sceneDockWidget->setIcon(QIcon::fromTheme("video-television-symbolic"));
    scenePreviewWidget = new ImageViewer(scene);
    scenePreviewWidget->mainWindow = this;
    connect(scenePreviewWidget, &ImageViewer::rectPicked, this,
            &NewMainWindow::sceneRectPicked);
    sceneDockWidget->setWidget(scenePreviewWidget);

    ads::CDockWidget *timelineDockWidget =
        dockManager->createDockWidget("Timeline");
    timeline = new TimelineWidget(scene, this);
    timelineDockWidget->setWidget(timeline);

    propertiesDockWidget = dockManager->createDockWidget("Properties");
    propertiesDockWidget->setIcon(
        QIcon::fromTheme("settings-configure-symbolic"));
    PropertyWindow *propertyWindow = new PropertyWindow(scene);
    propertiesDockWidget->setWidget(propertyWindow);

    ads::CDockWidget *effectsDockWidget =
        dockManager->createDockWidget("Effects");
    effectsDockWidget->setIcon(QIcon::fromTheme("special-effects-symbolic"));
    EffectsWindow *effectsWindow = new EffectsWindow(scene);
    effectsDockWidget->setWidget(effectsWindow);

    auto sceneDockArea = dockManager->addDockWidget(
        ads::DockWidgetArea::CenterDockWidgetArea, sceneDockWidget);
    auto propertiesDockArea = dockManager->addDockWidget(
        ads::DockWidgetArea::RightDockWidgetArea, propertiesDockWidget);

    auto effectsDockArea =
        dockManager->addDockWidget(ads::DockWidgetArea::BottomDockWidgetArea,
                                   effectsDockWidget, propertiesDockArea);
    auto timelineDockArea = dockManager->addDockWidget(
        ads::DockWidgetArea::BottomDockWidgetArea, timelineDockWidget);

    QSizePolicy policy = sceneDockArea->sizePolicy();
    policy.setHorizontalStretch(1);
    policy.setVerticalStretch(1);
    sceneDockArea->setSizePolicy(policy);
    policy = propertiesDockArea->sizePolicy();
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(1);
    propertiesDockArea->setSizePolicy(policy);
    policy = timelineDockArea->sizePolicy();
    policy.setHorizontalStretch(1);
    policy.setVerticalStretch(0);
    timelineDockArea->setSizePolicy(policy);

    dockManager->setSplitterSizes(sceneDockArea, {0, 350});

    QAction *saveLayoutAction = viewMenu->addAction("Save layout");
    saveLayoutAction->setToolTip(
        "This layout will be automatically loaded when the program is opened.");
    connect(saveLayoutAction, &QAction::triggered, this,
            &NewMainWindow::saveLayout);
    viewMenu->addSeparator();
    viewMenu->addAction(sceneDockWidget->toggleViewAction());
    viewMenu->addAction(timelineDockWidget->toggleViewAction());
    viewMenu->addAction(propertiesDockWidget->toggleViewAction());
    viewMenu->addAction(effectsDockWidget->toggleViewAction());

    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &NewMainWindow::about);
    QAction *aboutQtAction = helpMenu->addAction("About Qt");
    connect(aboutQtAction, &QAction::triggered, this, &NewMainWindow::aboutQt);

    QSettings settings;
    dockManager->loadPerspectives(settings);
    dockManager->openPerspective("Default");

    for (int i = 0; i < std::max(1, QThread::idealThreadCount() - 1); i++) {
        createThread();
    }

    connect(goToStartAction, &QAction::triggered, timeline,
            &TimelineWidget::goToStart);
    connect(playbackAction, &QAction::triggered, timeline,
            &TimelineWidget::togglePlay);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this,
            &NewMainWindow::clipboardContentsChanged);

    showWelcome(true);

    clipboardContentsChanged();
    elementSelectionChanged({});
    playbackStateChanged(false);

    rerender(false);
}

void NewMainWindow::importClicked() {
    QMimeDatabase db;
    QList<QString> imageExtensions;
    QList<QString> videoExtensions;
    for (auto mimeType : db.allMimeTypes()) {
        QString name = mimeType.name();
        if (name.startsWith("image/")) {
            imageExtensions << mimeType.suffixes();
        } else if (name.startsWith("video/")) {
            videoExtensions << mimeType.suffixes();
        }
    }

    imageExtensions.sort(Qt::CaseInsensitive);
    videoExtensions.sort(Qt::CaseInsensitive);

    QString filterString;

    filterString += "Image files (";
    for (const auto &extension : imageExtensions) {
        filterString += "*." + extension + " ";
    }
    filterString += ");;";

    filterString += "Video files (";
    for (const auto &extension : videoExtensions) {
        filterString += "*." + extension + " ";
    }
    filterString += ");;";

    QString mediaFilter = "Media files (";
    int index = 0;
    for (const auto &extension : imageExtensions) {
        if (index++ > 0)
            mediaFilter += " ";
        mediaFilter += "*." + extension;
    }
    for (const auto &extension : videoExtensions) {
        mediaFilter += " *." + extension;
    }
    mediaFilter += ")";

    filterString += mediaFilter;
    filterString += ";;";
    filterString += "All files (*.*)";

    QString selectedFilter = mediaFilter;
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Import files", "", filterString, &selectedFilter);

    if (files.isEmpty())
        return;

    // TODO: undo as group instead of individually
    for (const auto &file : files) {
        QMimeType mimeType = db.mimeTypeForFile(file);
        bool isVideo = mimeType.name().startsWith("video/");
        QString name = file.split("/").last().split("\\").last();

        Element *element;

        std::string path = file.toStdString();
        if (isVideo) {
            auto videoData = globalLoader.loadVideo(path);
            if (videoData->error) {
                KMessageBox::error(this,
                                   "Error while loading video\n\n" + file);
                continue;
            } else {
                float seconds = videoData->streamDuration;
                VideoElement *videoElement = new VideoElement();
                videoElement->x.set(0, {0});
                videoElement->y.set(0, {0});
                videoElement->w.set(scene->width, {0});
                videoElement->h.set(scene->height, {0});
                videoElement->path.set(path, {0});
                videoElement->durationFrames =
                    videoData->durationSeconds * scene->frameRate;
                element = videoElement;
            }
        } else {
            QImage img(file);
            if (img.isNull()) {
                KMessageBox::error(this,
                                   "Error while loading video\n\n" + file);
                continue;
            }
            ImageElement *imageElement = new ImageElement();
            imageElement->x.set(0, {0});
            imageElement->y.set(0, {0});
            imageElement->w.set(img.width(), {0});
            imageElement->h.set(img.height(), {0});
            imageElement->path.set(path, {0});
            element = imageElement;
        }

        element->setObjectName(name);
        addElementUndoable(element);
    }
}

void NewMainWindow::about() {
    QMessageBox::about(this, "About",
                       "Graphics Creator v" + VERSION +
                           "<br><br><a "
                           "href=\"https://github.com/chocolateimage/"
                           "graphics-creator\">Source code</a> | <a "
                           "href=\"https://github.com/chocolateimage/"
                           "graphics-creator/issues\">Report issue</a>");
}

void NewMainWindow::aboutQt() { QMessageBox::aboutQt(this); }

void NewMainWindow::groupSlot() {
    if (scene->selectedElements.isEmpty())
        return;

    Element *firstSelected = scene->selectedElements.first();
    int firstIndex = scene->elements.indexOf(firstSelected);
    GroupElement *groupElement = new GroupElement();

    int groupNumber = 1;
    while (true) {
        QString newName = "Group " + QString::number(groupNumber);
        bool existingName = false;
        for (auto element : scene->elements) {
            if (element->objectName() == newName) {
                existingName = true;
                break;
            }
        }
        if (existingName) {
            groupNumber++;
            continue;
        }

        groupElement->setObjectName(newName);
        break;
    }

    groupElement->setParent(firstSelected->getParent());
    for (auto element : scene->selectedElements) {
        element->setParent(groupElement->id);
    }
    scene->insertElement(groupElement, firstIndex);
    scene->selectElements({groupElement});
}

void NewMainWindow::ungroupSlot() {
    if (scene->selectedElements.isEmpty())
        return;

    for (auto element : scene->selectedElements) {
        GroupElement *groupElement = dynamic_cast<GroupElement *>(element);
        if (groupElement) {
            groupElement->ungroup();
        }
    }
}

void NewMainWindow::welcomeOpenClicked(const QString &path, bool asTemplate) {
    loadFile(path, asTemplate);
}

void NewMainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);

#ifdef UPDATE_CHECK
    QSettings settings;

    if (!settings.value("welcome/shown").toBool()) {
        WelcomePopup *welcomePopup = new WelcomePopup(this);
        welcomePopup->exec();
    }

    if (settings.value("updates/enabled").toBool()) {
        bool shouldCheck = true;
        if (settings.contains("updates/lastTimeChecked")) {
            qulonglong lastTime =
                settings.value("updates/lastTimeChecked").toULongLong();
            int secondsPassed =
                (QDateTime::currentDateTime().toSecsSinceEpoch() - lastTime);
            shouldCheck = secondsPassed >= 24 * 60 * 60;
        }
        if (shouldCheck) {
            settings.setValue("updates/lastTimeChecked",
                              QDateTime::currentDateTime().toSecsSinceEpoch());
            checkForUpdates();
        }
    }
#endif
}

void NewMainWindow::checkForUpdates() {
    qInfo() << "Checking for updates";
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.github.com/repos/chocolateimage/"
                        "graphics-creator/releases/latest"));
    request.setRawHeader("User-Agent",
                         "Mozilla/5.0 (X11; Linux x86_64; rv:153.0) "
                         "Gecko/20100101 Firefox/153.0");

    QNetworkReply *reply = networkAccessManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        int statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode >= 400) {
            qInfo() << "Error checking for updates. Status code is"
                    << statusCode;
            reply->deleteLater();
            return;
        }
        auto read = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(read);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QString latestVersion = obj["name"].toString();
            if (latestVersion != "v" + VERSION) {
                qInfo() << "Not latest version. Latest:" << latestVersion
                        << "Current:" << VERSION;
                notLatestVersion(obj);
            } else {
                qInfo() << "On latest version";
            }
        }
        reply->deleteLater();
    });
}

void NewMainWindow::notLatestVersion(QJsonObject obj) {
    if (KMessageBox::questionTwoActions(
            this,
            "<b>Update available</b><br><br>Current version: v" + VERSION +
                "<br>Latest version: " + obj["name"].toString() +
                "<br><br>Would you like to update?",
            "Update available", KGuiItem("Update"),
            KGuiItem("Remind me later")) != KMessageBox::PrimaryAction) {
        return;
    }

    for (const auto &asset : obj["assets"].toArray()) {
        const auto &assetObj = asset.toObject();
        QString assetName = assetObj["name"].toString();
#ifdef Q_OS_WIN
        if (assetName.contains("setup") && assetName.endsWith(".exe")) {
            qInfo() << "Found setup name:" << assetName;
            QString tempPath =
                QDir::tempPath() + "/" +
                QString::number(QRandomGenerator::global()->generate()) + "-" +
                assetName;
            QFile *tempFile = new QFile(tempPath);
            if (tempFile->open(QFile::WriteOnly)) {
                QProgressDialog *progressDialog =
                    new QProgressDialog("Downloading...", "", 0, 100, this);
                progressDialog->setCancelButton(nullptr);
                progressDialog->setAutoClose(false);
                progressDialog->show();
                QNetworkRequest request;
                request.setUrl(
                    QUrl(assetObj["browser_download_url"].toString()));
                request.setRawHeader(
                    "User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:153.0) "
                                  "Gecko/20100101 Firefox/153.0");

                QNetworkReply *reply = networkAccessManager->get(request);
                connect(
                    reply, &QNetworkReply::readyRead, this,
                    [tempFile, reply]() { tempFile->write(reply->readAll()); });
                connect(
                    reply, &QNetworkReply::downloadProgress, this,
                    [progressDialog](qint64 bytesReceived, qint64 bytesTotal) {
                        progressDialog->setMaximum(bytesTotal);
                        progressDialog->setValue(bytesReceived);
                    });
                connect(reply, &QNetworkReply::finished, this,
                        [tempPath, tempFile, reply]() {
                            tempFile->close();
                            reply->deleteLater();
                            QProcess::startDetached(tempPath, {"/VERYSILENT"});
                            qApp->exit();
                        });
                return;
            }
        }
#endif
    }

    QDesktopServices::openUrl(QUrl(obj["html_url"].toString()));
}

bool NewMainWindow::event(QEvent *event) {
    if (event->type() == QEvent::PaletteChange) {
        if (dockManager) {
            // workaround for docking library not updating palette when only
            // colors have changed
            dockManager->setColorSchemeMode(
                ads::CDockManager::ColorSchemeMode::Dark);
            dockManager->setColorSchemeMode(
                ads::CDockManager::ColorSchemeMode::Light);
            dockManager->setColorSchemeMode(
                ads::CDockManager::ColorSchemeMode::FollowPalette);
        }
    }
    return QMainWindow::event(event);
}

void NewMainWindow::showWelcome(bool show) {
    editMenu->setDisabled(show);
    videoMenu->setDisabled(show);
    viewMenu->setDisabled(show);
    saveAction->setDisabled(show);
    saveAsAction->setDisabled(show);
    toolBar->setHidden(show);
    toolBar->setDisabled(show);
    statusBar->setHidden(show);
    if (show) {
        mainStackWidget->setCurrentIndex(0);
        dockManager->hideManagerAndFloatingWidgets();
        setWindowTitle("");
    } else {
        mainStackWidget->setCurrentIndex(1);
        dockManager->show();
        setOpenFilePath("");
    }
}

bool NewMainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == toolBar->widgetForAction(controlRectangle)) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            sceneRectPicked("", {0, 0, scene->width, scene->height});
            scenePreviewWidget->stopPicking();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void NewMainWindow::saveLayout() {
    QSettings settings;
    dockManager->addPerspective("Default");
    dockManager->savePerspectives(settings);
}

void NewMainWindow::previousFrameSlot() {
    scene->setFramesChanging(true);
    scene->setFrame(scene->currentFrame - 1);
    scene->setFramesChanging(false);
}

void NewMainWindow::nextFrameSlot() {
    scene->setFramesChanging(true);
    scene->setFrame(scene->currentFrame + 1);
    scene->setFramesChanging(false);
}

void NewMainWindow::previousKeyframeSlot() {
    int latestFrame = 0;
    for (const auto &keyframe : timeline->timelineContent->keyframeData) {
        int kFrame = keyframe.keyframe->frame;
        if (kFrame < scene->currentFrame && kFrame > latestFrame) {
            latestFrame = kFrame;
        }
    }
    scene->setFramesChanging(true);
    scene->setFrame(latestFrame);
    scene->setFramesChanging(false);
}

void NewMainWindow::nextKeyframeSlot() {
    int latestFrame = scene->durationFrames - 1;
    for (const auto &keyframe : timeline->timelineContent->keyframeData) {
        int kFrame = keyframe.keyframe->frame;
        if (kFrame > scene->currentFrame && kFrame < latestFrame) {
            latestFrame = kFrame;
        }
    }
    scene->setFramesChanging(true);
    scene->setFrame(latestFrame);
    scene->setFramesChanging(false);
}

void NewMainWindow::clipboardContentsChanged() {
    const QMimeData *data = QApplication::clipboard()->mimeData();
    if (!data) {
        return;
    }
    pasteAction->setEnabled(data->hasFormat(ELEMENT_COPY_MIME_TYPE));
}

void NewMainWindow::copySlot() {
    QJsonArray array;
    for (auto element : scene->selectedElements) {
        QJsonObject elementObj = element->serialize();
        elementObj.remove("id");
        array.append(elementObj);
    }

    QJsonDocument doc;
    doc.setArray(array);

    QMimeData *mimeData = new QMimeData();
    mimeData->setData(ELEMENT_COPY_MIME_TYPE,
                      doc.toJson(QJsonDocument::Compact));
    QApplication::clipboard()->setMimeData(mimeData);
}

void NewMainWindow::pasteSlot() {
    const QMimeData *data = QApplication::clipboard()->mimeData();
    if (!data->hasFormat(ELEMENT_COPY_MIME_TYPE))
        return;

    auto byteData = data->data(ELEMENT_COPY_MIME_TYPE);
    QJsonDocument doc = QJsonDocument::fromJson(byteData);
    QJsonArray array = doc.array();
    QList<Element *> newSelected;
    QUndoCommand *command{nullptr};
    for (auto elementValue : array) {
        QJsonObject elementObj = elementValue.toObject();
        Element *element = loadElementFromJson(elementObj);
        if (!element) {
            continue;
        }
        scene->insertElement(element, 0);
        newSelected.append(element);
    }
    scene->selectElements(newSelected);
}

void NewMainWindow::duplicateSlot() {
    QList<QJsonObject> array;
    for (auto element : scene->selectedElements) {
        QJsonObject elementObj = element->serialize();
        elementObj.remove("id");
        array.append(elementObj);
    }

    QList<Element *> newSelected;
    QUndoCommand *command{nullptr};
    for (auto elementObj : array) {
        Element *element = loadElementFromJson(elementObj);
        if (!element) {
            continue;
        }
        scene->insertElement(element, 0);
        newSelected.append(element);
    }
    scene->selectElements(newSelected);
}

void NewMainWindow::closeEvent(QCloseEvent *event) {
    if (askSaveConfirmation()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool NewMainWindow::askSaveConfirmation() {
    if (!isWindowModified())
        return true;

    KMessageBox::ButtonCode result = KMessageBox::warningTwoActionsCancel(
        this, "Your unsaved changes will be lost. Do you want to save changes?",
        "Unsaved changes", KStandardGuiItem::save(),
        KStandardGuiItem::dontSave(), KStandardGuiItem::cancel());
    if (result == KMessageBox::ButtonCode::PrimaryAction) {
        return saveSlot();
    } else if (result == KMessageBox::ButtonCode::SecondaryAction) {
        return true;
    } else {
        return false;
    }
}

void NewMainWindow::openVideoSettings() {
    VideoSettingsDialog *dialog = new VideoSettingsDialog(scene, this);
    dialog->open();
}

void NewMainWindow::openSlot() {
    if (!askSaveConfirmation())
        return;

    QString filePath = QFileDialog::getOpenFileName(
        this, "Open Project", "", "Graphics Creator Project (*.gcp)");
    if (filePath.isEmpty())
        return;

    loadFile(filePath, false);
}

void NewMainWindow::loadFile(const QString &filePath, bool asTemplate) {
    QFileInfo fileInfo{filePath};
    QString newPath = fileInfo.absoluteFilePath();
    QFile file(newPath);
    if (!file.open(QFile::ReadOnly)) {
        KMessageBox::error(
            this, "Could not open file for reading\n\n" + file.errorString(),
            "Error loading");
        return;
    }

    showWelcome(false);
    if (asTemplate) {
        setOpenFilePath("");
    } else {
        setOpenFilePath(newPath);
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    loadFrom(doc);

    if (asTemplate) {
        setWindowModified(true);
    }
}

bool NewMainWindow::saveSlot() {
    if (openFilePath.isEmpty()) {
        return saveAsSlot();
    } else {
        return save();
    }
}

bool NewMainWindow::saveAsSlot() {
    QString filePath = QFileDialog::getSaveFileName(
        this, "Save Project", "", "Graphics Creator Project (*.gcp)");
    if (filePath.isEmpty())
        return false;
    setOpenFilePath(filePath);
    save();
    return true;
}

bool NewMainWindow::save() {
    QFile file(openFilePath);
    if (!file.open(QFile::WriteOnly)) {
        KMessageBox::error(
            this, "Could not open file for writing\n\n" + file.errorString(),
            "Error saving");
        return false;
    }

    QJsonDocument doc = saveInto();
    doc.toJson();
    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();
    setWindowModified(false);

    QDir appData(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    if (appData.mkpath("previews")) {
        QImage scaled = scenePreviewWidget->image.scaled(
            PREVIEW_IMAGE_WIDTH, PREVIEW_IMAGE_HEIGHT,
            Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(openFilePath.toUtf8());
        QString filePath = appData.absoluteFilePath(
            "previews/" + hash.result().toHex() + ".png");
        scaled.save(filePath);
    }

    return true;
}

void NewMainWindow::setOpenFilePath(const QString &newPath) {
    openFilePath = newPath;
    if (openFilePath.isEmpty()) {
        setWindowTitle("Untitled Project [*]");
    } else {
        setWindowTitle(QFileInfo(openFilePath).completeBaseName() + " [*]");
        QSettings settings;
        QStringList recents = settings.value("recent/list").toStringList();
        recents.removeAll(newPath);
        recents.append(newPath);
        while (recents.size() > 10) {
            recents.removeFirst();
        }
        settings.setValue("recent/list", recents);
    }
}

QJsonDocument NewMainWindow::saveInto() {
    QJsonObject rootObject;

    rootObject["version"] = 1;

    QJsonObject sceneObject;
    sceneObject["width"] = scene->width;
    sceneObject["height"] = scene->height;
    sceneObject["durationFrames"] = scene->durationFrames;
    sceneObject["frameRate"] = scene->frameRate;
    sceneObject["currentFrame"] = scene->currentFrame;
    QJsonArray elementsArray;
    for (auto element : scene->elements) {
        elementsArray.append(element->serialize());
    }
    sceneObject["elements"] = elementsArray;
    rootObject["scene"] = sceneObject;

    QJsonDocument document;
    document.setObject(rootObject);
    return document;
}

void NewMainWindow::newSlot() {
    if (!askSaveConfirmation())
        return;

    QSettings settings;
    Scene *tempScene = new Scene();
    tempScene->width = settings.value("scene/width", 1280).toInt();
    tempScene->height = settings.value("scene/height", 720).toInt();
    tempScene->frameRate = settings.value("scene/frameRate", 30).toDouble();
    tempScene->durationFrames =
        settings.value("scene/durationFrames", 150).toInt();
    VideoSettingsDialog *dialog = new VideoSettingsDialog(tempScene, this);
    if (!dialog->exec()) {
        delete tempScene;
        return;
    }

    settings.setValue("scene/width", tempScene->width);
    settings.setValue("scene/height", tempScene->height);
    settings.setValue("scene/frameRate", tempScene->frameRate);
    settings.setValue("scene/durationFrames", tempScene->durationFrames);

    newProjectNew(tempScene->width, tempScene->height, tempScene->frameRate,
                  tempScene->durationFrames);
    delete tempScene;
}

void NewMainWindow::newProjectNew(int width, int height, double frameRate,
                                  int durationFrames) {
    newProject(width, height, frameRate, durationFrames);
    showWelcome(false);

    inProgressTasks.clear();
    renderingFrames.clear();
    scene->setFramesChanging(true);
    scene->setFrame(0);
    scene->setFramesChanging(false);

    scene->undoStack->clear();
    invalidateAndRerender();
    setWindowModified(false);
}

void NewMainWindow::newProject(int width, int height, double frameRate,
                               int durationFrames) {
    scene->selectElements({});
    scene->stopTimer();

    scene->width = width;
    scene->height = height;
    scene->frameRate = frameRate;
    scene->durationFrames = durationFrames;

    for (auto element : scene->elements) {
        delete element;
    }
    scene->elements.clear();
    timeline->updateContents();
}

bool NewMainWindow::loadFrom(const QJsonDocument &document) {
    if (!document.isObject()) {
        return false;
    }

    QJsonObject rootObject = document.object();
    int docVersion = rootObject["version"].toInt(-1);
    if (docVersion == -1) {
        return false;
    }

    QJsonObject sceneObject = rootObject["scene"].toObject();
    newProject(sceneObject["width"].toInt(), sceneObject["height"].toInt(),
               sceneObject["frameRate"].toDouble(),
               sceneObject["durationFrames"].toInt());
    int newFrame = sceneObject["currentFrame"].toInt();

    for (auto elementValue : sceneObject["elements"].toArray()) {
        QJsonObject elementObj = elementValue.toObject();
        Element *element = loadElementFromJson(elementObj);
        if (!element) {
            continue;
        }
        scene->addElement(element);
    }

    inProgressTasks.clear();
    renderingFrames.clear();
    emit scene->sceneInfoChanged();
    scene->setFramesChanging(true);
    scene->setFrame(newFrame);
    scene->setFramesChanging(false);
    setWindowModified(false);

    scene->undoStack->clear();
    return true;
}

Element *NewMainWindow::loadElementFromJson(const QJsonObject &obj) {
    QString elementType = obj["elementType"].toString();
    Element *element{nullptr};
    if (elementType == "rectangle") {
        element = new RectangleElement();
    } else if (elementType == "ellipse") {
        element = new EllipseElement();
    } else if (elementType == "text") {
        element = new TextElement();
    } else if (elementType == "image") {
        element = new ImageElement();
    } else if (elementType == "group") {
        element = new GroupElement();
    } else if (elementType == "video") {
        element = new VideoElement();
    }

    if (!element) {
        qWarning() << "Invalid element type" << elementType;
        KMessageBox::error(this, "Invalid element type " + elementType);
        return element;
    }

    element->deserialize(obj);
    return element;
}

void NewMainWindow::openRenderWindow() {
    if (!renderWindow) {
        renderWindow = new RenderWindow(this);
    }
    renderWindow->show();
    renderWindow->activateWindow();
    renderWindow->resetRenderFilePathInput();
}

void NewMainWindow::playbackStateChanged(bool playing) {
    if (!playing) {
        statusText->setText("");
    }

    if (playing) {
        playbackAction->setIcon(QIcon::fromTheme("media-playback-pause"));
        playbackAction->setText("Pause");
    } else {
        playbackAction->setText("Play");
        playbackAction->setIcon(QIcon::fromTheme("media-playback-start"));
    }
}

void NewMainWindow::loadDefaultFont() {
    const FcChar8 *fontToMatch =
        (const FcChar8 *)"Noto Sans,Arial,DejaVu Sans:regular:slant=0";

    FcPattern *pattern = FcNameParse(fontToMatch);
    FcResult result;
    FcPattern *font = FcFontMatch(nullptr, pattern, &result);
    if (result != FcResultMatch || !font) {
        FcPatternDestroy(pattern);
        return;
    }

    FcChar8 *rawFileName;
    FcChar8 *rawFamily;
    FcChar8 *rawStyle;
    int fontIndex;
    FcPatternGetString(font, FC_FILE, 0, &rawFileName);
    FcPatternGetInteger(font, FC_INDEX, 0, &fontIndex);
    FcPatternGetString(font, FC_FAMILY, 0, &rawFamily);
    FcPatternGetString(font, FC_STYLE, 0, &rawStyle);

    Variant::defaultFont = {std::string((char *)rawFileName), fontIndex,
                            std::string((char *)rawFamily) + " " +
                                std::string((char *)rawStyle),
                            std::string((char *)fontToMatch)};

    FcPatternDestroy(font);

    FcPatternDestroy(pattern);
}

void NewMainWindow::elementSelectionChanged(QList<Element *> elements) {
    copyAction->setDisabled(elements.isEmpty());
    duplicateAction->setDisabled(elements.isEmpty());
    deleteAction->setDisabled(elements.isEmpty());
    groupAction->setDisabled(elements.isEmpty());
    bool hasGroupElement = false;
    for (auto element : elements) {
        if (dynamic_cast<GroupElement *>(element)) {
            hasGroupElement = true;
        }
    }
    ungroupAction->setEnabled(hasGroupElement);
}

void NewMainWindow::deleteTriggered() {
    if (timeline->timelineContent->deleteSelected())
        return;

    QList<Element *> elementsToDelete;

    for (auto element : scene->selectedElements) {
        elementsToDelete.append(element);
        elementsToDelete.append(element->getDescendants());
    }

    scene->undoStack->push(new RemoveElementsCommand(scene, elementsToDelete));
}

void NewMainWindow::createThread() {
    FramePreviewThread *thread = new FramePreviewThread(this);
    thread->window = this;
    connect(thread, &FramePreviewThread::taskDone, this,
            &NewMainWindow::taskCompleted);
    previewThreads.append(thread);

    connect(thread, &FramePreviewThread::finished, thread, [this, thread]() {
        previewThreads.removeOne(thread);
        thread->deleteLater();
    });

    thread->start();
}

void NewMainWindow::taskCompleted(FrameTask *task) {
    if (!inProgressTasks.contains(task)) {
        delete[] task->values;
        delete task;
        return;
    }
    inProgressTasks.removeOne(task);
    auto it = savedFrames.find(task->frame);
    SavedFrame savedFrame{
        .values = task->values,
        .id = task->id,
    };
    if (it != savedFrames.end()) {
        if (it->second.id > task->id) {
            delete[] task->values;
            delete task;
            return;
        }

        delete[] it->second.values;
        it->second = savedFrame;
    } else {
        savedFrames.emplace(task->frame, savedFrame);
    }
    renderingFrames.remove(task->frame);
    if (task->frame == scene->currentFrame) {
        updatePreview();
    }
    delete task;
    timeline->timelineContent->update();
}

void NewMainWindow::controlsUpdated() {
    if (controlSelect->isChecked()) {
        scenePreviewWidget->stopPicking();
    } else {
        scenePreviewWidget->beginPicking("", "", ImageViewer::Rect);
    }
}

void NewMainWindow::addElementUndoable(Element *element) {
    scene->undoStack->push(new AddElementCommand(scene, element));
}

void NewMainWindow::sceneRectPicked(QString id, QRect rect) {
    Element *element{nullptr};
    if (controlRectangle->isChecked()) {
        controlSelect->setChecked(true);
        if (!rect.isEmpty()) {
            RectangleElement *rectElement = new RectangleElement();
            rectElement->setObjectName("New Rectangle");
            element = rectElement;
        }
    } else if (controlEllipse->isChecked()) {
        controlSelect->setChecked(true);
        if (!rect.isEmpty()) {
            EllipseElement *ellipseElement = new EllipseElement();
            ellipseElement->setObjectName("New Ellipse");
            element = ellipseElement;
        }
    } else if (controlText->isChecked()) {
        controlSelect->setChecked(true);
        if (!rect.isEmpty()) {
            if (rect.width() < 40 || rect.height() < 40) {
                rect.setWidth(1);
                rect.setHeight(1);
            }
            TextElement *textElement = new TextElement();
            textElement->setObjectName("New Text");
            element = textElement;
        }
    }
    if (element) {
        element->x.set(rect.x(), {0});
        element->y.set(rect.y(), {0});
        element->w.set(rect.width(), {0});
        element->h.set(rect.height(), {0});
        addElementUndoable(element);
        if (dynamic_cast<TextElement *>(element)) {
            element->setEditMode(true);
        }
    }
}

void NewMainWindow::elementAdded(Element *element, int index) {
    invalidateAndRerender();
}

void NewMainWindow::elementOrderChanged() { invalidateAndRerender(); }

void NewMainWindow::elementUpdated(Element *element) {
    invalidateAndRerender();
}

void NewMainWindow::invalidateAndRerender() {
    for (auto &frame : savedFrames) {
        delete[] frame.second.values;
    }
    savedFrames.clear();

    setWindowModified(true);

    rerender(true);
    lastRenderDelayTimer.start();
}

void NewMainWindow::invalidateAndRerender_afterDelay() {
    for (int i = scene->currentFrame + 1;
         i < std::min(scene->durationFrames, scene->currentFrame + 50); i++) {
        invalidateFrame(i);
        createTask(i);
    }
    for (int i = std::max(0, scene->currentFrame - 50); i < scene->currentFrame;
         i++) {
        invalidateFrame(i);
        createTask(i);
    }
}

void NewMainWindow::invalidateFrame(int frame) {
    auto it = savedFrames.find(frame);
    if (it == savedFrames.end())
        return;

    delete[] it->second.values;
    savedFrames.erase(it);
}

void NewMainWindow::frameChanged(int frame) { rerender(false); }

void NewMainWindow::rerender(bool onlyCurrentFrame) {
    if (!createTask(scene->currentFrame)) {
        updatePreview();
    }

    if (!onlyCurrentFrame) {
        for (int i = scene->currentFrame + 1;
             i < std::min(scene->durationFrames,
                          scene->currentFrame + (int)previewThreads.length());
             i++) {
            createTask(i % scene->durationFrames);
        }
    }
}

bool NewMainWindow::createTask(int frame) {
    auto it = savedFrames.find(frame);
    if (it != savedFrames.end()) {
        return false;
    }

    renderingFrames.insert(frame);

    FrameTask *task = new FrameTask();
    task->width = scene->width;
    task->height = scene->height;
    task->frame = frame;
    task->seconds = frame / scene->frameRate;
    task->id = ++globalId;

    inProgressTasks.append(task);

    for (auto element : scene->elements) {
        ElementRender *render = (ElementRender *)element->toRender({frame});
        task->renderElements.insert(task->renderElements.begin(), render);
    }

    {
        QMutexLocker lock(&openTasksMutex);
        for (int i = 0; i < openTasks.length(); i++) {
            if (openTasks[i]->frame == frame) {
                delete openTasks[i];
                openTasks.removeAt(i);
                i--;
            }
        }
        openTasks.append(task);
    }

    return true;
}

void NewMainWindow::updatePreview() {
    auto it = savedFrames.find(scene->currentFrame);
    if (it == savedFrames.end()) {
        return;
    }

    QImage img = QImage((uchar *)it->second.values, scene->width, scene->height,
                        scene->width * 4, QImage::Format_ARGB32)
                     .copy();

    scenePreviewWidget->updateImage(img);
}

NewMainWindow::~NewMainWindow() {
    for (auto thread : previewThreads) {
        thread->stayAlive = false;
    }
    for (auto thread : previewThreads) {
        thread->wait();
    }
    for (auto frame : savedFrames) {
        delete[] frame.second.values;
    }
    delete scene;
    if (renderWindow) {
        delete renderWindow;
    }
}

int main(int argc, char **argv) {
#ifdef Q_OS_WIN
    CreateMutexA(nullptr, false, "GraphicsCreatorOpen");
#endif
    KIconTheme::initTheme();
    QApplication application(argc, argv);
    application.setOrganizationName(QStringLiteral("graphics-creator"));
    application.setApplicationName(QStringLiteral("graphics-creator"));
    application.setDesktopFileName("me.chocolateimage.graphics-creator");
    application.setApplicationDisplayName(QStringLiteral("Graphics Creator"));
    application.setApplicationVersion(VERSION);
    KStyleManager::initStyle();

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("filepath", "Project to open", "[filepath]");

    QCommandLineOption newProjectOption(
        "new",
        "Create a new project without showing the welcome screen. Format: "
        "width:height:durationFrames:fps. Example: 1920:1080:300:60",
        "info");
    parser.addOption(newProjectOption);

    QCommandLineOption renderOption("render",
                                    "Render project into file. Will not "
                                    "overwrite unless --overwrite is set. An "
                                    "encoder with --encoder must be set.",
                                    "file");
    parser.addOption(renderOption);

    QCommandLineOption overwriteOption(QStringList() << "y" << "overwrite",
                                       "Overwrite file when rendering");
    parser.addOption(overwriteOption);

    QCommandLineOption encoderOption(
        "encoder",
        "The FFmepg encoder to use when rendering. For mov with transparency: "
        "prores (or prores_ks which may be faster but can cause issues). For "
        "mp4/H264: libx264. For mp4/H264 with NVDIA: h264_nvenc. "
        "For webm/VP9: libvpx-vp9. List of encoders can be viewed with ffmpeg "
        "-encoders",
        "encoder");
    parser.addOption(encoderOption);

    parser.process(application);

    const QStringList args = parser.positionalArguments();
    QString newProject = parser.value(newProjectOption);
    QString renderFile = parser.value(renderOption);

    NewMainWindow widget;
    if (!args.isEmpty()) {
        widget.loadFile(args[0], false);
    } else if (!newProject.isEmpty()) {
        QStringList splitted = newProject.split(":");
        if (splitted.length() != 4) {
            qCritical() << "Invalid format for --new";
            return 1;
        }
        int width = splitted[0].toInt();
        int height = splitted[1].toInt();
        int durationFrames = splitted[2].toInt();
        int fps = splitted[3].toInt();
        widget.newProjectNew(width, height, fps, durationFrames);
    }

    if (!renderFile.isEmpty()) {
        qInfo() << "";
        QString encoder = parser.value(encoderOption);
        if (encoder.isEmpty()) {
            qCritical()
                << "An encoder must also be set with --encoder. View the help "
                   "with --help to see a short list of encoders.";
            return 1;
        }
        QFileInfo info(renderFile);
        if (info.exists()) {
            if (!parser.isSet(overwriteOption)) {
                qCritical()
                    << "Not overwriting file" << info.absoluteFilePath();
                qCritical() << "Use --overwrite to overwrite the file";
                return 1;
            }
        }

        // TODO: template/placeholder text

        RenderWindow *renderWindow = new RenderWindow(&widget);
        renderWindow->renderFilePathInput->setText(info.absoluteFilePath());
        renderWindow->render(info, encoder);
        if (renderWindow->thread) {
            renderWindow->thread->wait();
            if (renderWindow->thread->hasErrored) {
                qCritical() << "An error occured while rendering:"
                            << qPrintable(renderWindow->thread->errorMsg);
            }
            return 0;
        } else {
            return 1;
        }
    } else {
        widget.show();
    }

    return application.exec();
}
