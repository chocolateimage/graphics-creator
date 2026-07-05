#pragma once
#include "animatable/element/element.hpp"
#include <QList>
#include <QObject>

class Scene : public QObject {
    Q_OBJECT
  public:
    int width;
    int height;
    QList<Element *> elements;
    QList<Element *> selectedElements;

    void addElement(Element *element);
    void insertElement(Element *element, int index);
    void selectElements(QList<Element *> elements);

  signals:
    void elementAdded(Element *element, int index);
    void elementRemoved(Element *element);
    void elementSelectionChanged(QList<Element *> elements);
};
