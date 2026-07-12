#pragma once
#include "scene.hpp"
#include <QMenu>
#include <QVBoxLayout>
#include <QWidget>

class EffectsWindow : public QWidget {
    Q_OBJECT
  public:
    explicit EffectsWindow(Scene *scene);
    Scene *scene;
    QVBoxLayout *mainLayout;

    QMenu *createEffectsMenu(QWidget *parent);

    QMetaObject::Connection effectUpdateConnection;

  public slots:
    void selectedElementsUpdated(QList<Element *> selectedElements);
    void framesChanging(bool changing);
    void addEffectTriggered(QAction *action);
    void effectListUpdated();
};
