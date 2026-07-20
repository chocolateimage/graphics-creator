#include "gui.hpp"
#include "animatable/element/ellipse_element.hpp"
#include "animatable/element/rectangle_element.hpp"
#include "animatable/element/text_element.hpp"
#include "effects_window.hpp"
#include "math.hpp"
#include "property_window.hpp"
#include "render.hpp"
#include "timeline.hpp"
#include "variant.hpp"
#include <DockAreaWidget.h>
#include <KIconTheme>
#include <KStyleManager>
#include <QActionGroup>
#include <QApplication>
#include <QElapsedTimer>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <fontconfig/fontconfig.h>

FramePreviewTask::~FramePreviewTask() {
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
        FramePreviewTask *task;
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
        uint32_t *frame = new uint32_t[task->width * task->height];
        memset(frame, 0, task->width * task->height * 4);

        for (auto element : task->renderElements) {
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
                Rect effectBox = effect->getRenderBox(finalRect);
                effect->currentFrame = task->frame;
                effect->currentSeconds = task->seconds;
                effect->renderBox = effectBox;
                uint32_t *effectValues =
                    new uint32_t[effectBox.w * effectBox.h];
                memset(effectValues, 0, effectBox.w * effectBox.h * 4);

                bool success =
                    effect->render(finalValues, finalRect, effectValues);
                if (!success) {
                    delete[] effectValues;
                    continue;
                }

                delete[] finalValues;
                finalValues = effectValues;
                finalRect = effectBox;
            }

            int maxY =
                std::min(task->height, finalRect.y + finalRect.h) - finalRect.y;
            int maxX =
                std::min(task->width, finalRect.x + finalRect.w) - finalRect.x;

            for (int y = std::max(0, -finalRect.y); y < maxY; y++) {
                for (int x = std::max(0, -finalRect.x); x < maxX; x++) {
                    auto index = pixelIndex(x + finalRect.x, y + finalRect.y,
                                            task->width);
                    frame[index] =
                        over(frame[index],
                             finalValues[pixelIndex(x, y, finalRect.w)]);
                }
            }

            delete[] finalValues;
        }

        task->values = frame;

        for (auto element : task->renderElements) {
            delete element;
        }
        task->renderElements.clear();

        // qInfo() << "Render time:"
        //         << qPrintable(QString("%1").arg(
        //                renderTime.nsecsElapsed() / 1000000., 0, 'f', 1))
        //         << "ms";

        emit taskDone(task);
    }

    renderThread.close();
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
    undoStack->setUndoLimit(3);
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
    deleteAction = editMenu->addAction("Delete");
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setIcon(QIcon::fromTheme("delete"));
    connect(deleteAction, &QAction::triggered, this,
            &NewMainWindow::deleteTriggered);

    QMenu *viewMenu = menuBar->addMenu("View");
    setMenuBar(menuBar);

    QToolBar *toolBar = addToolBar("Controls");
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

    dockManager = new ads::CDockManager(this);

    ads::CDockWidget *sceneDockWidget = dockManager->createDockWidget("Scene");
    sceneDockWidget->setIcon(QIcon::fromTheme("video-television-symbolic"));
    scenePreviewWidget = new ImageViewer(scene);
    connect(scenePreviewWidget, &ImageViewer::rectPicked, this,
            &NewMainWindow::sceneRectPicked);
    sceneDockWidget->setWidget(scenePreviewWidget);

    ads::CDockWidget *timelineDockWidget =
        dockManager->createDockWidget("Timeline");
    timeline = new TimelineWidget(scene, this);
    timelineDockWidget->setWidget(timeline);

    ads::CDockWidget *propertiesDockWidget =
        dockManager->createDockWidget("Properties");
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
    auto elementsDockArea = dockManager->addDockWidget(
        ads::DockWidgetArea::RightDockWidgetArea, propertiesDockWidget);
    auto effectsDockArea =
        dockManager->addDockWidget(ads::DockWidgetArea::BottomDockWidgetArea,
                                   effectsDockWidget, elementsDockArea);
    auto timelineDockArea = dockManager->addDockWidget(
        ads::DockWidgetArea::BottomDockWidgetArea, timelineDockWidget);

    QSizePolicy policy = sceneDockArea->sizePolicy();
    policy.setHorizontalStretch(1);
    policy.setVerticalStretch(1);
    sceneDockArea->setSizePolicy(policy);
    policy = elementsDockArea->sizePolicy();
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(1);
    elementsDockArea->setSizePolicy(policy);
    policy = timelineDockArea->sizePolicy();
    policy.setHorizontalStretch(1);
    policy.setVerticalStretch(0);
    timelineDockArea->setSizePolicy(policy);

    viewMenu->addAction(sceneDockWidget->toggleViewAction());
    viewMenu->addAction(timelineDockWidget->toggleViewAction());
    viewMenu->addAction(propertiesDockWidget->toggleViewAction());
    viewMenu->addAction(effectsDockWidget->toggleViewAction());

    for (int i = 0; i < std::max(1, QThread::idealThreadCount() - 1); i++) {
        createThread();
    }

    rerender();
}

void NewMainWindow::playbackStateChanged(bool playing) {
    if (!playing) {
        statusText->setText("");
    }
}

void NewMainWindow::loadDefaultFont() {
    const FcChar8 *fontsToMatch[] = {
        (const FcChar8 *)"Noto Sans:regular:slant=0",
        (const FcChar8 *)"Arial:regular:slant=0",
    };

    for (auto fontName : fontsToMatch) {
        FcPattern *pattern = FcNameParse(fontName);
        FcResult result;
        FcPattern *font = FcFontMatch(nullptr, pattern, &result);
        if (result != FcResultMatch || !font) {
            FcPatternDestroy(pattern);
            continue;
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

        break;
    }
}

void NewMainWindow::elementSelectionChanged(QList<Element *> elements) {
    deleteAction->setEnabled(!elements.isEmpty());
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

void NewMainWindow::taskCompleted(FramePreviewTask *task) {
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
        undoStack->push(new AddElementCommand(scene, element));
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
    for (int i = 0; i < scene->durationFrames; i++) {
        invalidateFrame(i);
    }
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

    FramePreviewTask *task = new FramePreviewTask();
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

void NewMainWindow::invalidateFrame(int frame) {
    auto it = savedFrames.find(frame);
    if (it == savedFrames.end())
        return;

    delete[] it->second.values;
    savedFrames.erase(it);
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
