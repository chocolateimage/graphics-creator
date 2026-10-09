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
    QList<Preset *> presets;

  private slots:
    void doubleClicked(QListWidgetItem *item);
};
