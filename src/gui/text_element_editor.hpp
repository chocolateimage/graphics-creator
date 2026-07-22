#pragma once
#include "animatable/element/text_element.hpp"
#include "brush_input.hpp"
#include "fontcombobox.hpp"
#include "scene.hpp"
#include <DockWidget.h>
#include <QCheckBox>
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

    void repaintParent();

    TextLayout layout;

    TextElement *textElement;
    Scene *scene;
    NewMainWindow *mainWindow;
    int selectionStart{0};
    int selectionLength{0};
    bool selectionAnchorLeft = false;

    ads::CDockWidget *dockWidget;

    void loadValues();
    void loadValues(TextSpan &span);
    QWidget *dockContentWidget;
    QSpinBox *fontSize;
    FontComboBox *fontComboBox;
    BrushInput *fillInput;
    QCheckBox *antialiasedCheckBox;
    QSpinBox *strokeWidth;
    BrushInput *strokeInput;
    QComboBox *strokeLineJoin;
    QListWidget *debugListWidget;

    void setSpanProperties(std::function<void(TextSpan &)> func);

    TextSpan tempSpan{};

    bool debug{false};

  private slots:
    void setFont();
    void setFontSize(int newValue);
    void setFill(Brush value);
    void setAntialiased(bool newValue);
    void setStrokeWidth(int newValue);
    void setStroke(Brush value);
    void setStrokeLineJoin(int value);
};
