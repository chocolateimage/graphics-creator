#include "gui.hpp"
#include "animatable/element/ellipse_element.hpp"
#include "animatable/element/rectangle_element.hpp"
#include "effects_window.hpp"
#include "math.hpp"
#include "property_window.hpp"
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

class TestCommand : public QUndoCommand {
  public:
    TestCommand() { setText("add rectangle"); }
    void undo() override { qInfo() << "UNDO"; }
    void redo() override { qInfo() << "REDO"; }
};

NewMainWindow::NewMainWindow() : QMainWindow() {
    undoStack = new QUndoStack(this);
    scene = new Scene();
    scene->width = 1280;
    scene->height = 720;
    scene->frameRate = 30;
    scene->durationFrames = scene->frameRate * 5;

    connect(scene, &Scene::elementUpdated, this,
            &NewMainWindow::elementUpdated);
    connect(scene, &Scene::frameChanged, this, &NewMainWindow::frameChanged);

    this->resize(1200, 700);

    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize,
                                     true);
    ads::CDockManager::setAutoHideConfigFlags(
        ads::CDockManager::DefaultAutoHideConfig);

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
    controlLua = toolBar->addAction(QIcon::fromTheme("scriptnew"), "Lua");

    editMenu->addSeparator();

    editMenu->addAction(controlSelect);
    editMenu->addAction(controlRectangle);
    editMenu->addAction(controlEllipse);
    editMenu->addAction(controlPolygon);
    editMenu->addAction(controlLua);

    controlSelect->setActionGroup(controlsGroup);
    controlRectangle->setActionGroup(controlsGroup);
    controlEllipse->setActionGroup(controlsGroup);
    controlPolygon->setActionGroup(controlsGroup);
    controlLua->setActionGroup(controlsGroup);

    controlSelect->setCheckable(true);
    controlRectangle->setCheckable(true);
    controlEllipse->setCheckable(true);
    controlPolygon->setCheckable(true);
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
    TimelineWidget *timeline = new TimelineWidget(scene);
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

    rerender();
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
    }
    if (element) {
        element->x.set(rect.x(), {0});
        element->y.set(rect.y(), {0});
        element->w.set(rect.width(), {0});
        element->h.set(rect.height(), {0});
        scene->insertElement(element, 0);
        scene->selectElements({element});
    }
    rerender();
}

void NewMainWindow::elementUpdated(Element *element) { rerender(); }
void NewMainWindow::frameChanged(int frame) { rerender(); }

void NewMainWindow::rerender() { renderTest(); }

void NewMainWindow::renderTest() {
    // TODO: this is bad this is bad this is bad
    // This should be in a render thread. The data (like elements) should
    // probably be copied to avoid crashing.
    // and instead of splitting a single frame into chunks, I will probably do
    // a whole frame per thread.

    QElapsedTimer renderTime;
    renderTime.start();
    uint32_t *frame = new uint32_t[scene->width * scene->height];
    memset(frame, 0, scene->width * scene->height * 4);

    std::vector<ElementRender *> renderElements;

    for (auto element : scene->elements) {
        ElementRender *render =
            (ElementRender *)element->toRender({scene->currentFrame});
        renderElements.insert(renderElements.begin(), render);
    }

    for (auto element : renderElements) {
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
            effect->currentFrame = scene->currentFrame;
            effect->currentSeconds =
                (double)scene->currentFrame / scene->frameRate;
            effect->renderBox = effectBox;
            uint32_t *effectValues = new uint32_t[effectBox.w * effectBox.h];
            memset(effectValues, 0, effectBox.w * effectBox.h * 4);

            bool success = effect->render(finalValues, finalRect, effectValues);
            if (!success) {
                delete[] effectValues;
                continue;
            }

            delete[] finalValues;
            finalValues = effectValues;
            finalRect = effectBox;
        }

        int maxY =
            std::min(scene->height, finalRect.y + finalRect.h) - finalRect.y;
        int maxX =
            std::min(scene->width, finalRect.x + finalRect.w) - finalRect.x;

        for (int y = std::max(0, -finalRect.y); y < maxY; y++) {
            for (int x = std::max(0, -finalRect.x); x < maxX; x++) {
                auto index =
                    pixelIndex(x + finalRect.x, y + finalRect.y, scene->width);
                frame[index] = over(frame[index],
                                    finalValues[pixelIndex(x, y, finalRect.w)]);
            }
        }

        delete[] finalValues;
    }

    for (auto element : renderElements) {
        delete element;
    }
    qInfo() << "Render time:"
            << qPrintable(QString("%1").arg(
                   renderTime.nsecsElapsed() / 1000000., 0, 'f', 1))
            << "ms";

    QImage img = QImage((unsigned char *)frame, scene->width, scene->height,
                        scene->width * 4, QImage::Format_ARGB32)
                     .copy();
    scenePreviewWidget->updateImage(img);
    delete[] frame;
}

NewMainWindow::~NewMainWindow() { delete scene; }

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
