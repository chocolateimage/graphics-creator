#pragma once
#include "animatable/element/text_element.hpp"
#include "scene.hpp"
#include <QKeyEvent>
#include <QObject>

class TextElementEditor : public QObject {
  public:
    TextElementEditor(Scene *scene, TextElement *textElement, QObject *parent);
    ~TextElementEditor();

    void relayout();
    void passKeyEvent(QKeyEvent *keyEvent);
    void paint(QPainter &painter);
    QRect getRectForIndex(int index);

    QList<QRect> spanRects;

    FontManager *fontManager;
    TextElement *textElement;
    Scene *scene;
    int selectionStart{0};
    int selectionLength{0};
    bool selectionAnchorLeft = false;
};
