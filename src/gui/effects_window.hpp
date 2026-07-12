#pragma once
#include "scene.hpp"
#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

class EffectsWindow : public QWidget {
    Q_OBJECT
  public:
    explicit EffectsWindow(Scene *scene);
    Scene *scene;
    QStackedWidget *stackedWidget;
    QWidget *mainWidget;
    QLabel *errorLabel;
    QVBoxLayout *topMainLayout;
    QVBoxLayout *mainLayout;
    QScrollArea *scrollArea;
    QWidget *scrollContents;
    QLabel *effectCountLabel;

    QVBoxLayout *effectsLayout;

    QMenu *createEffectsMenu(QWidget *parent);

    QMetaObject::Connection effectUpdateConnection;

  public slots:
    void selectedElementsUpdated(QList<Element *> selectedElements);
    void framesChanging(bool changing);
    void addEffectTriggered(QAction *action);
    void effectListUpdated();
};
