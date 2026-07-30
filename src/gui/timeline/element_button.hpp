#pragma once
#include "animatable/element/element.hpp"
#include <QLabel>
#include <QPushButton>

class TimelineWidget;

class TimelineElementButton : public QPushButton {
  public:
    explicit TimelineElementButton(Element *element,
                                   TimelineWidget *timelineWidget);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private slots:
    void elementNameChanged(const QString &objectName);
    void collapseClicked();
    void clickedSlot();
    void visibilityClicked();
    void visibilityUpdated();

  private:
    TimelineWidget *timelineWidget;
    Element *element;
    QPoint dragStartPosition;
    QLabel *objectNameLabel;
    QPushButton *visibilityButton;
};
