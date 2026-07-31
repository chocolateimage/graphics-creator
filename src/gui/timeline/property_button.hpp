#pragma once
#include "animatable/property.hpp"
#include "scene.hpp"
#include <QPushButton>

class TimelineWidget;

class TimelinePropertyButton : public QPushButton {
  public:
    explicit TimelinePropertyButton(PropertyBase *property, Scene *scene,
                                    TimelineWidget *timelineWidget, bool stripe,
                                    QPushButton *elementButton, int indent,
                                    bool showEdit);

  private slots:
    void toggleAnimationClicked();
    void addAnimationClicked();
    void previousKeyframeClicked();
    void keyframeClicked();
    void nextKeyframeClicked();

    void updateKeyframe();
    void propertyUpdated(PropertyBase *updatedProperty);
    void updateAnimating(PropertyBase *updatedProperty);

  private:
    TimelineWidget *timelineWidget;
    PropertyBase *property;
    Scene *scene;

    QPushButton *toggleAnimationButton;
    QWidget *keyframeRightSideSpacing;
    QPushButton *previousButton;
    QPushButton *keyframeButton;
    QPushButton *nextButton;
};
