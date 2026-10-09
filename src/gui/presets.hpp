#pragma once
#include "scene.hpp"
#include <QListWidget>
#include <QWidget>

class NewMainWindow;
class TextElement;

const QString PRESET_MIME_TYPE =
    QStringLiteral("application/x-graphicscreator-preset");

class Preset {
  public:
    Preset(QString displayName, QString id)
        : displayName(displayName), id(id) {}
    virtual ~Preset() {}
    QString displayName;
    QString id;
    virtual bool canApply(Element *element) = 0;
    virtual void apply(Element *element) = 0;
};

class TextPreset : public Preset {
  public:
    TextPreset(QString displayName, QString id,
               std::function<void(TextElement *)> applyFunc)
        : Preset(displayName, id), applyFunc(applyFunc) {}

    std::function<void(TextElement *)> applyFunc;

    bool canApply(Element *element) override;
    void apply(Element *element) override;
};

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
