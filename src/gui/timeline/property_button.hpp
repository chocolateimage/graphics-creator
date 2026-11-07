#pragma once
#include "animatable/property.hpp"
#include "scene.hpp"
#include <QPushButton>

class TimelineWidget;

class TimelinePropertyButton : public QPushButton {
  public:
    explicit TimelinePropertyButton(PropertyBase *property, Scene *scene,
                                    TimelineWidget *timelineWidget,
                                    QPushButton *elementButton, int indent,
                                    bool showEdit);

  private slots:
    void resetClicked();
    void toggleAnimationClicked();
    void addAnimationClicked();
    void previousKeyframeClicked();
    void keyframeClicked();
    void nextKeyframeClicked();

    void updateKeyframe();
    void propertyUpdated(PropertyBase *updatedProperty);
    void updateAnimating(PropertyBase *updatedProperty);

    void overlayUpdate();

  private:
    bool showEdit;

    TimelineWidget *timelineWidget;
    PropertyBase *property;
    Scene *scene;

    QPushButton *toggleAnimationButton;
    QWidget *keyframeRightSideSpacing;
    QPushButton *previousButton;
    QPushButton *keyframeButton;
    QPushButton *nextButton;

    QWidget *resetButtonSpacing{nullptr};
    QPushButton *resetButton{nullptr};

    QWidget *overlayWidget{nullptr};
    float overlayOpacity{0};
    QTimer overlayTimer;
};
