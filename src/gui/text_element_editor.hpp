#pragma once
#include "animatable/element/text_element.hpp"
#include "scene.hpp"
#include <DockWidget.h>
#include <QKeyEvent>
#include <QListWidget>
#include <QObject>
#include <QSpinBox>

class NewMainWindow;

class TextElementEditor : public QObject {
  public:
    TextElementEditor(NewMainWindow *mainWindow, Scene *scene,
                      TextElement *textElement, QObject *parent);
    ~TextElementEditor();

    void relayout();
    void passKeyEvent(QKeyEvent *keyEvent);
    void paint(QPainter &painter);

    TextLayout layout;

    TextElement *textElement;
    Scene *scene;
    NewMainWindow *mainWindow;
    int selectionStart{0};
    int selectionLength{0};
    bool selectionAnchorLeft = false;

    ads::CDockWidget *dockWidget;

    void loadValues();
    QWidget *dockContentWidget;
    QSpinBox *fontSize;
    QListWidget *debugListWidget;

  private slots:
    void setFontSize(int newValue);
};
