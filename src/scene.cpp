#include "scene.hpp"

Scene::Scene() {
    timer = new QChronoTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(std::chrono::milliseconds(3));
    connect(timer, &QChronoTimer::timeout, this, &Scene::timerTicked);
}

Scene::~Scene() { qDeleteAll(elements); }

void Scene::addElement(Element *element) {
    insertElement(element, elements.length());
}

void Scene::insertElement(Element *element, int index) {
    elements.insert(index, element);
    emit elementAdded(element, index);
    connect(element, &Element::propertyUpdated, this,
            [this, element]() { emit elementUpdated(element); });
    connect(element, &Element::effectPropertyUpdated, this,
            [this, element]() { emit elementUpdated(element); });
    connect(element, &Element::effectListUpdated, this,
            [this, element]() { emit elementUpdated(element); });
}

void Scene::selectElements(QList<Element *> elements) {
    selectedElements = elements;
    emit elementSelectionChanged(elements);
}

void Scene::startTimer() {
    startFrame = currentFrame;
    elapsedTimer.restart();
    emit playbackStateChanged(true);
    emit framesChanging(true);
    timer->start();
}

void Scene::stopTimer() {
    timer->stop();
    emit playbackStateChanged(false);
    emit framesChanging(false);
}

void Scene::timerTicked() {
    int msecElapsed = elapsedTimer.elapsed();
    int framesElapsed = msecElapsed / 1000.0 * frameRate;
    int newFrame = startFrame + framesElapsed;

    if (currentFrame == newFrame) {
        return;
    }

    if (newFrame >= durationFrames) {
        newFrame = 0;
        startFrame = 0;
        elapsedTimer.restart();
    }

    if (!canContinuePlayback()) {
        startFrame = currentFrame;
        elapsedTimer.restart();
        return;
    }

    setFrame(newFrame);
}

void Scene::setFrame(int frame) {
    int newFrame = std::max(0, std::min(frame, durationFrames - 1));
    if (currentFrame != newFrame) {
        currentFrame = newFrame;
        emit frameChanged(currentFrame);
    }
}

bool Scene::isPlaying() { return timer->isActive(); }
