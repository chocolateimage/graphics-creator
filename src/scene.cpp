#include "scene.hpp"

Scene::Scene() {
    timer = new QChronoTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(std::chrono::milliseconds(3));
    connect(timer, &QChronoTimer::timeout, this, &Scene::timerTicked);
}

Scene::~Scene() { qDeleteAll(elements); }

void Scene::setFramesChanging(bool changing) { emit framesChanging(changing); }

void Scene::addElement(Element *element) {
    insertElement(element, elements.length());
}

void Scene::insertElement(Element *element, int index) {
    if (element->durationFrames == INT32_MIN) {
        element->durationFrames = durationFrames - element->startFrame;
    }
    element->scene = this;
    elements.insert(index, element);
    emit elementAdded(element, index);
    connect(element, &Element::propertyUpdated, this,
            &Scene::_elementUpdatedSlot);
    connect(element, &Element::effectListUpdated, this,
            &Scene::_elementUpdatedSlot);
    connect(element, &Element::visibilityUpdated, this,
            &Scene::_elementUpdatedSlot);
    connect(element, &Element::editModeUpdated, this,
            &Scene::_elementEditModeChangedSlot);
}

void Scene::_elementUpdatedSlot() { emit elementUpdated((Element *)sender()); }

void Scene::_elementEditModeChangedSlot(bool editMode) {
    emit elementEditModeChanged((Element *)sender(), editMode);
}

void Scene::selectElements(QList<Element *> elements) {
    for (auto element : selectedElements) {
        element->setEditMode(false);
    }
    selectedElements = elements;
    emit elementSelectionChanged(elements);
}

void Scene::removeElement(Element *element) {
    element->scene = nullptr;
    element->setEditMode(false);
    disconnect(element, &Element::propertyUpdated, this,
               &Scene::_elementUpdatedSlot);
    disconnect(element, &Element::effectListUpdated, this,
               &Scene::_elementUpdatedSlot);
    disconnect(element, &Element::visibilityUpdated, this,
               &Scene::_elementUpdatedSlot);
    disconnect(element, &Element::editModeUpdated, this,
               &Scene::_elementEditModeChangedSlot);

    elements.removeOne(element);
    emit elementRemoved(element);
}

void Scene::reorderElement(Element *element, int newIndex) {
    int oldIndex = elements.indexOf(element);
    if (newIndex == oldIndex)
        return;

    int toAddIndex = newIndex;
    if (toAddIndex > oldIndex) {
        toAddIndex--;
    }
    elements.removeOne(element);
    elements.insert(toAddIndex, element);

    emit elementOrderChanged();
}

void Scene::startTimer() {
    if (timer->isActive())
        return;
    startFrame = currentFrame;
    elapsedTimer.restart();
    emit playbackStateChanged(true);
    setFramesChanging(true);
    timer->start();
}

void Scene::stopTimer() {
    if (!timer->isActive())
        return;

    timer->stop();
    emit playbackStateChanged(false);
    setFramesChanging(false);
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
