#pragma once
#include "scene.hpp"
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

class EffectsWindow;

class EffectWidget : public QWidget {
    Q_OBJECT
  public:
    explicit EffectWidget(Scene *scene, Element *element, Effect *effect,
                          QWidget *parent);
    QPushButton *effectButton;
    QPushButton *collapseButton;
    QPushButton *enabledButton;
    QWidget *propertiesWidget;
    Scene *scene;
    Element *element;
    Effect *effect;

    QPoint dragStartPosition;

    void deleteClick();
    void collapseClick();
    void collapsedChanged();
    void enabledClick();

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

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

    QFrame *effectMoveBar{nullptr};
    int effectMoveTarget;

    QList<EffectWidget *> effectWidgets;

  protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  public slots:
    void selectedElementsUpdated(QList<Element *> selectedElements);
    void framesChanging(bool changing);
    void addEffectTriggered(QAction *action);
    void effectListUpdated();
};
