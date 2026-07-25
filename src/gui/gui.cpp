#include "gui.hpp"
#include "animatable/element/ellipse_element.hpp"
#include "animatable/element/image_element.hpp"
#include "animatable/element/rectangle_element.hpp"
#include "animatable/element/text_element.hpp"
#include "effects_window.hpp"
#include "math.hpp"
#include "property_window.hpp"
#include "render.hpp"
#include "render_window.hpp"
#include "timeline.hpp"
#include "variant.hpp"
#include <DockAreaWidget.h>
#include <KIconTheme>
#include <KMessageBox>
#include <KStyleManager>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QToolBar>
#include <fontconfig/fontconfig.h>

const QString ELEMENT_COPY_MIME_TYPE =
    "application/x-graphicscreator-element-copy";

void FrameTask::render(RenderThread &renderThread) {
    uint32_t *frameValues = new uint32_t[width * height];
    memset(frameValues, 0, width * height * 4);

    for (auto element : renderElements) {
        element->renderThread = &renderThread;
        element->prepare();
        auto rect = element->getRenderBox();
        uint32_t *elementValues = new uint32_t[rect.w * rect.h];
        memset(elementValues, 0, rect.w * rect.h * 4);

        Rect finalRect = rect;
        uint32_t *finalValues = elementValues;

        bool success = element->render(elementValues);

        if (!success) {
            delete[] elementValues;
            continue;
        }

        for (auto effect : element->effects) {
            effect->currentFrame = frame;
            effect->currentSeconds = seconds;
            effect->originalBox = rect;
            effect->originalValues = elementValues;
            Rect effectBox = effect->getRenderBox(finalRect);
            effect->renderBox = effectBox;
            uint32_t *effectValues = new uint32_t[effectBox.w * effectBox.h];
            memset(effectValues, 0, effectBox.w * effectBox.h * 4);

            bool success = effect->render(finalValues, finalRect, effectValues);
            if (!success) {
                delete[] effectValues;
                continue;
            }

            if (elementValues != finalValues) {
                delete[] finalValues;
            }
            finalValues = effectValues;
            finalRect = effectBox;
        }

        int maxY = std::min(height, finalRect.y + finalRect.h) - finalRect.y;
        int maxX = std::min(width, finalRect.x + finalRect.w) - finalRect.x;

        for (int y = std::max(0, -finalRect.y); y < maxY; y++) {
            for (int x = std::max(0, -finalRect.x); x < maxX; x++) {
                auto index =
                    pixelIndex(x + finalRect.x, y + finalRect.y, width);
                frameValues[index] =
                    over(frameValues[index],
                         finalValues[pixelIndex(x, y, finalRect.w)]);
            }
        }

        if (elementValues != finalValues) {
            delete[] finalValues;
        }
        delete[] elementValues;
    }

    values = frameValues;

    for (auto element : renderElements) {
        delete element;
    }
    renderElements.clear();
}

FrameTask::~FrameTask() {
    for (auto element : renderElements) {
        delete element;
    }
    renderElements.clear();
}

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

    frameRate = new QSpinBox(this);
    frameRate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frameRate->setRange(1, 999999);

    duration = new QDoubleSpinBox(this);
    duration->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    duration->setRange(0, 999999);
    duration->setSuffix(" seconds");

    durationFramesLabel = new QLabel(this);

    width->setValue(scene->width);
    height->setValue(scene->height);
    frameRate->setValue(scene->frameRate);
    duration->setValue((double)scene->durationFrames / scene->frameRate);
    connect(frameRate, &QSpinBox::valueChanged, this,
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
    scene->frameRate = frameRate->value();
    scene->width = width->value();
    scene->height = height->value();
    scene->durationFrames = (int)(duration->value() * frameRate->value());
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
    undoStack = new QUndoStack(this);
    undoStack->setUndoLimit(20);
    scene = new Scene();
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

    QStatusBar *statusBar = new QStatusBar(this);
    statusText = new QLabel(statusBar);
    statusBar->addPermanentWidget(statusText);
    setStatusBar(statusBar);

    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *fileMenu = menuBar->addMenu("File");

    QAction *openAction = fileMenu->addAction("Open…");
    openAction->setIcon(QIcon::fromTheme("document-open-data"));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &NewMainWindow::openSlot);

    QAction *saveAction = fileMenu->addAction("Save");
    saveAction->setIcon(QIcon::fromTheme("document-save"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &NewMainWindow::saveSlot);

    QAction *saveAsAction = fileMenu->addAction("Save as…");
    saveAsAction->setIcon(QIcon::fromTheme("document-save"));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this,
            &NewMainWindow::saveAsSlot);

    fileMenu->addSeparator();

    QAction *quitAction = fileMenu->addAction("Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(quitAction, &QAction::triggered, this, &NewMainWindow::close);

    QMenu *editMenu = menuBar->addMenu("Edit");
    QAction *undoAction = undoStack->createUndoAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAction);
    QAction *redoAction = undoStack->createRedoAction(this);
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

    deleteAction = editMenu->addAction("Delete");
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setIcon(QIcon::fromTheme("delete"));
    connect(deleteAction, &QAction::triggered, this,
            &NewMainWindow::deleteTriggered);

    QMenu *videoMenu = menuBar->addMenu("Video");

    QAction *goToStartAction = videoMenu->addAction("Go to start");
    goToStartAction->setIcon(QIcon::fromTheme("media-skip-backward"));

    playbackAction = videoMenu->addAction("");
    playbackAction->setShortcut(QKeySequence(" "));

    videoMenu->addSeparator();

    QAction *videoSettingsAction = videoMenu->addAction("Settings…");
    videoSettingsAction->setIcon(QIcon::fromTheme("settings-configure"));
    connect(videoSettingsAction, &QAction::triggered, this,
            &NewMainWindow::openVideoSettings);

    QMenu *viewMenu = menuBar->addMenu("View");
    setMenuBar(menuBar);

    QToolBar *toolBar = addToolBar("Controls");
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    QActionGroup *controlsGroup = new QActionGroup(toolBar);
    controlSelect = toolBar->addAction(QIcon::fromTheme("select"), "Select");
    controlSelect->setShortcut(QKeySequence("V"));
    toolBar->addSeparator();
    controlRectangle =
        toolBar->addAction(QIcon::fromTheme("draw-rectangle"), "Rectangle");
    controlRectangle->setShortcut(QKeySequence("R"));
    controlEllipse =
        toolBar->addAction(QIcon::fromTheme("draw-circle"), "Ellipse");
    controlEllipse->setShortcut(QKeySequence("E"));
    controlPolygon =
        toolBar->addAction(QIcon::fromTheme("draw-polygon"), "Polygon");
    controlText = toolBar->addAction(QIcon::fromTheme("draw-text"), "Text");
    controlText->setShortcut(QKeySequence("T"));
    controlLua = toolBar->addAction(QIcon::fromTheme("scriptnew"), "Lua");

    // not implemented yet
    controlPolygon->setVisible(false);
    controlLua->setVisible(false);

    editMenu->addSeparator();

    editMenu->addAction(controlSelect);
    editMenu->addAction(controlRectangle);
    editMenu->addAction(controlEllipse);
    editMenu->addAction(controlPolygon);
    editMenu->addAction(controlText);
    editMenu->addAction(controlLua);

    controlSelect->setActionGroup(controlsGroup);
    controlRectangle->setActionGroup(controlsGroup);
    controlEllipse->setActionGroup(controlsGroup);
    controlPolygon->setActionGroup(controlsGroup);
    controlText->setActionGroup(controlsGroup);
    controlLua->setActionGroup(controlsGroup);

    controlSelect->setCheckable(true);
    controlRectangle->setCheckable(true);
    controlEllipse->setCheckable(true);
    controlPolygon->setCheckable(true);
    controlText->setCheckable(true);
    controlLua->setCheckable(true);

    controlSelect->setChecked(true);

    connect(controlSelect, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlRectangle, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlEllipse, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlPolygon, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlText, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);
    connect(controlLua, &QAction::triggered, this,
            &NewMainWindow::controlsUpdated);

    QWidget *spacing = new QWidget(toolBar);
    spacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->addWidget(spacing);

    renderAction =
        toolBar->addAction(QIcon::fromTheme("media-record"), "Render…");
    connect(renderAction, &QAction::triggered, this,
            &NewMainWindow::openRenderWindow);

    dockManager = new ads::CDockManager(this);

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

    viewMenu->addAction(sceneDockWidget->toggleViewAction());
    viewMenu->addAction(timelineDockWidget->toggleViewAction());
    viewMenu->addAction(propertiesDockWidget->toggleViewAction());
    viewMenu->addAction(effectsDockWidget->toggleViewAction());

    for (int i = 0; i < std::max(1, QThread::idealThreadCount() - 1); i++) {
        createThread();
    }

    connect(goToStartAction, &QAction::triggered, timeline,
            &TimelineWidget::goToStart);
    connect(playbackAction, &QAction::triggered, timeline,
            &TimelineWidget::togglePlay);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this,
            &NewMainWindow::clipboardContentsChanged);

    clipboardContentsChanged();
    elementSelectionChanged({});
    playbackStateChanged(false);
    setOpenFilePath("");
    rerender();
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
        array.append(element->serialize());
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

    setOpenFilePath(filePath);

    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        KMessageBox::error(
            this, "Could not open file for reading\n\n" + file.errorString(),
            "Error loading");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    loadFrom(doc);
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
    return true;
}

void NewMainWindow::setOpenFilePath(const QString &newPath) {
    openFilePath = newPath;
    if (openFilePath.isEmpty()) {
        setWindowTitle("Untitled Project [*]");
    } else {
        setWindowTitle(openFilePath + " [*]");
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

bool NewMainWindow::loadFrom(const QJsonDocument &document) {
    if (!document.isObject()) {
        return false;
    }

    QJsonObject rootObject = document.object();
    int docVersion = rootObject["version"].toInt(-1);
    if (docVersion == -1) {
        return false;
    }

    scene->selectElements({});
    scene->stopTimer();

    QJsonObject sceneObject = rootObject["scene"].toObject();
    scene->width = sceneObject["width"].toInt();
    scene->height = sceneObject["height"].toInt();
    scene->durationFrames = sceneObject["durationFrames"].toInt();
    scene->frameRate = sceneObject["frameRate"].toDouble();
    int newFrame = sceneObject["currentFrame"].toInt();

    for (auto element : scene->elements) {
        delete element;
    }
    scene->elements.clear();
    timeline->updateContents();

    for (auto elementValue : sceneObject["elements"].toArray()) {
        QJsonObject elementObj = elementValue.toObject();
        Element *element = loadElementFromJson(elementObj);
        if (!element) {
            continue;
        }
        scene->addElement(element);
    }

    scene->setFramesChanging(true);
    scene->setFrame(newFrame);
    scene->setFramesChanging(false);
    setWindowModified(false);

    undoStack->clear();
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
    }

    if (!element) {
        qWarning() << "Invalid element type" << elementType;
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
                                std::string((char *)rawStyle)};

    FcPatternDestroy(font);

    FcPatternDestroy(pattern);
}

void NewMainWindow::elementSelectionChanged(QList<Element *> elements) {
    copyAction->setDisabled(elements.isEmpty());
    deleteAction->setDisabled(elements.isEmpty());
}

void NewMainWindow::deleteTriggered() {
    if (timeline->timelineContent->deleteSelected())
        return;

    undoStack->push(new RemoveElementsCommand(scene, scene->selectedElements));
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
    undoStack->push(new AddElementCommand(scene, element));
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
    rerender();
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

    rerender();
    for (int i = scene->currentFrame + 1;
         i < std::min(scene->durationFrames, scene->currentFrame + 50); i++) {
        createTask(i);
    }
    for (int i = std::max(0, scene->currentFrame - 50); i < scene->currentFrame;
         i++) {
        createTask(i);
    }
}

void NewMainWindow::frameChanged(int frame) { rerender(); }

void NewMainWindow::rerender() {
    if (!createTask(scene->currentFrame)) {
        updatePreview();
    }
    for (int i = scene->currentFrame + 1;
         i < scene->currentFrame + (int)previewThreads.length(); i++) {
        createTask(i % scene->durationFrames);
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
    KIconTheme::initTheme();
    QApplication application(argc, argv);
    qInfo() << "pid:" << application.applicationPid();
    application.setDesktopFileName("me.chocolateimage.graphics-creator");
    application.setApplicationDisplayName(QStringLiteral("Graphics Creator"));
    KStyleManager::initStyle();
    NewMainWindow widget;
    widget.show();
    return application.exec();
}
