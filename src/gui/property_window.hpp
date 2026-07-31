#pragma once
#include "scene.hpp"
#include <QFormLayout>
#include <QPushButton>
#include <QWidget>

class PropertyWindow : public QWidget {
    Q_OBJECT
  public:
    explicit PropertyWindow(Scene *scene);
    Scene *scene;
    QFormLayout *formLayout;

    void selectedElementsUpdated(QList<Element *> selectedElements);
    void framesChanging(bool changing);
};
