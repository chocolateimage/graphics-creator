#include "scene.hpp"

Scene::~Scene() { qDeleteAll(elements); }

void Scene::addElement(Element *element) {
    insertElement(element, elements.length());
}

void Scene::insertElement(Element *element, int index) {
    elements.insert(index, element);
    emit elementAdded(element, index);
    connect(element, &Element::propertyUpdated, this,
            [this, element]() { emit elementUpdated(element); });
}

void Scene::selectElements(QList<Element *> elements) {
    selectedElements = elements;
    emit elementSelectionChanged(elements);
}
