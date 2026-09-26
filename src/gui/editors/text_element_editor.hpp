#pragma once
#include "animatable/element/text_element.hpp"
#include "editor.hpp"
#include "gui/brush_input.hpp"
#include "gui/fontcombobox.hpp"
#include <DockWidget.h>
#include <QCheckBox>
#include <QKeyEvent>
#include <QListWidget>
#include <QObject>
#include <QSpinBox>

class NewMainWindow;

class TextElementEditor : public Editor {
    Q_OBJECT
  public:
    TextElementEditor(NewMainWindow *mainWindow, Scene *scene,
                      TextElement *textElement, ImageViewer *parent);
    ~TextElementEditor();

    void relayout();
    void passKeyEvent(QKeyEvent *keyEvent) override;
    void paint(QPainter &painter) override;
    bool shouldTransformPainter() override { return true; };

    TextLayout layout;

    TextElement *textElement;
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
    QDoubleSpinBox *strokeWidth;
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
    void setStrokeWidth(double newValue);
    void setStroke(Brush value);
    void setStrokeLineJoin(int value);
};
