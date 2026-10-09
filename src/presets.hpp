#pragma once
#include <QString>

const QString PRESET_MIME_TYPE =
    QStringLiteral("application/x-graphicscreator-preset");

class NewMainWindow;
class Element;
class TextElement;

void initPresets();

class Preset {
  public:
    Preset(QString displayName, QString id)
        : displayName(displayName), id(id) {}
    virtual ~Preset() {}
    QString displayName;
    QString id;
    virtual bool canApply(NewMainWindow *mainWindow, Element *element) = 0;
    virtual Element *apply(NewMainWindow *mainWindow, Element *element) = 0;
};

class TextPreset : public Preset {
  public:
    TextPreset(QString displayName, QString id,
               std::function<void(TextElement *)> applyFunc)
        : Preset(displayName, id), applyFunc(applyFunc) {}

    std::function<void(TextElement *)> applyFunc;

    bool canApply(NewMainWindow *mainWindow, Element *element) override;
    Element *apply(NewMainWindow *mainWindow, Element *element) override;
};

extern QList<Preset *> presetList;
