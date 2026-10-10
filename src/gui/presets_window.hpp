#pragma once
#include "presets.hpp"
#include "scene.hpp"
#include <QListWidget>
#include <QWidget>

class NewMainWindow;

class PresetsListWidget : public QListWidget {
    Q_OBJECT
  public:
    QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override;
};

class PresetsWindow : public QWidget {
    Q_OBJECT
  public:
    PresetsWindow(Scene *scene, NewMainWindow *mainWindow);
    Scene *scene;
    NewMainWindow *mainWindow;
    PresetsListWidget *listWidget;
    QList<Preset *> presets;

    void reloadPresets();

  protected:
    bool event(QEvent *event) override;

  private slots:
    void doubleClicked(QListWidgetItem *item);
};
