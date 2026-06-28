#include "scene.hpp"

void Scene::addElement(Element *element) {
    insertElement(element, elements.length());
}

void Scene::insertElement(Element *element, int index) {
    elements.insert(index, element);
    emit elementAdded(element, index);
}

void Scene::selectElements(QList<Element *> elements) {
    selectedElements = elements;
    emit elementSelectionChanged(elements);
}
