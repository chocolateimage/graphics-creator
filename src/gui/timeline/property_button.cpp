#include "property_button.hpp"
#include "animatable/element/text_element.hpp"
#include "gui/property_edit.hpp"
#include "timeline.hpp"
#include "timeline_content.hpp"
#include <QHBoxLayout>
#include <QLabel>

TimelinePropertyButton::TimelinePropertyButton(
    PropertyBase *property, Scene *scene, TimelineWidget *timelineWidget,
    bool stripe, QPushButton *elementButton, int indent, bool showEdit)
    : timelineWidget(timelineWidget), property(property), scene(scene) {
    this->setObjectName("property");
    QString background = "transparent";
    QString backgroundSelected = "rgba(128,128,128,0.1)";
    if (stripe) {
        background = "palette(alternate-base)";
        backgroundSelected = "rgba(128,128,128,0.13)";
    }
    this->setStyleSheet("#property {"
                        "   text-align: left;"
                        "   background: " +
                        background +
                        ";"
                        "   border-radius: 0px;"
                        "}"
                        "#property[flat=\"false\"] {"
                        "   background: " +
                        backgroundSelected +
                        ";"
                        "   border-left: 3px solid palette(accent);"
                        "}");
    this->setFixedHeight(PROPERTY_TRACK_HEIGHT);
    this->setFlat(elementButton->isFlat());
    connect(this, &QPushButton::clicked, elementButton, &QPushButton::click);

    QHBoxLayout *propertyLayout = new QHBoxLayout(this);
    propertyLayout->setContentsMargins(indent, 0, 0, 0);
    propertyLayout->setSpacing(0);

    toggleAnimationButton =
        new PropertyToggleAnimationButton(scene, property, this);
    toggleAnimationButton->setFixedWidth(32);
    propertyLayout->addWidget(toggleAnimationButton);

    QLabel *label = new QLabel(this);
    label->setText(property->getDisplayName());
    label->setFixedWidth(150 - indent);
    propertyLayout->addWidget(label);

    if (property->flags.contains("textElementText")) {
        QPushButton *addAnimation = new QPushButton();
        addAnimation->setFixedWidth(100);
        addAnimation->setFixedHeight(PROPERTY_TRACK_HEIGHT);
        addAnimation->setText("Animation");
        addAnimation->setIcon(QIcon::fromTheme("list-add"));
        connect(addAnimation, &QPushButton::clicked, this,
                &TimelinePropertyButton::addAnimationClicked);
        propertyLayout->addWidget(addAnimation);
    }

    if (showEdit) {
        PropertyEdit *edit = new PropertyEdit(property, scene, this);
        edit->setFixedHeight(PROPERTY_TRACK_HEIGHT);
        propertyLayout->addWidget(edit);
    }

    propertyLayout->addStretch();

    keyframeRightSideSpacing = new QWidget(this);
    keyframeRightSideSpacing->setFixedWidth(72);
    keyframeRightSideSpacing->setFixedHeight(2);
    propertyLayout->addWidget(keyframeRightSideSpacing);

    previousButton = new QPushButton(this);
    previousButton->setIcon(QIcon::fromTheme("arrow-left"));
    previousButton->setToolTip("Go to previous keyframe");
    previousButton->setFlat(true);
    previousButton->setFixedWidth(24);
    connect(previousButton, &QPushButton::clicked, this,
            &TimelinePropertyButton::previousKeyframeClicked);
    propertyLayout->addWidget(previousButton);

    keyframeButton = new QPushButton(this);
    keyframeButton->setToolTip("Toggle keyframe");
    keyframeButton->setFlat(true);
    keyframeButton->setFixedWidth(24);
    connect(keyframeButton, &QPushButton::clicked, this,
            &TimelinePropertyButton::keyframeClicked);
    propertyLayout->addWidget(keyframeButton);

    nextButton = new QPushButton(this);
    nextButton->setIcon(QIcon::fromTheme("arrow-right"));
    nextButton->setToolTip("Go to next keyframe");
    nextButton->setFlat(true);
    nextButton->setFixedWidth(24);
    connect(nextButton, &QPushButton::clicked, this,
            &TimelinePropertyButton::nextKeyframeClicked);
    propertyLayout->addWidget(nextButton);

    updateAnimating(property);

    connect(property->animatable, &Animatable::propertyIsAnimatingUpdated, this,
            &TimelinePropertyButton::updateAnimating);
    connect(property->animatable, &Animatable::propertyUpdated, this,
            &TimelinePropertyButton::propertyUpdated);
    connect(scene, &Scene::frameChanged, this,
            &TimelinePropertyButton::updateKeyframe);
}

void TimelinePropertyButton::addAnimationClicked() {
    TextElement *textElement = (TextElement *)property->animatable;
    TextAnimator *newAnimator = new TextAnimator(textElement);
    TextAnimatorSelector *selector = new TextAnimatorSelector(newAnimator);
    newAnimator->selectors.append(selector);
    textElement->textAnimators.append(newAnimator);
    emit textElement->effectListUpdated(); // hack
}

void TimelinePropertyButton::previousKeyframeClicked() {
    for (int index = property->keyframes.size() - 1; index >= 0; index--) {
        if (property->keyframes[index]->frame < scene->currentFrame) {
            scene->setFramesChanging(true);
            scene->setFrame(property->keyframes[index]->frame);
            scene->setFramesChanging(false);
            break;
        }
    }
}

void TimelinePropertyButton::keyframeClicked() {
    if (property->has(scene->currentFrame)) {
        property->remove(scene->currentFrame);
    } else {
        property->addToPosition({scene->currentFrame});
    }
    timelineWidget->timelineContent->updateContents();
}

void TimelinePropertyButton::nextKeyframeClicked() {
    for (auto keyframe : property->keyframes) {
        if (keyframe->frame > scene->currentFrame) {
            scene->setFramesChanging(true);
            scene->setFrame(keyframe->frame);
            scene->setFramesChanging(false);
            break;
        }
    }
}

void TimelinePropertyButton::updateKeyframe() {
    if (signalsBlocked())
        return;
    if (!property->isAnimating)
        return;

    if (property->has(scene->currentFrame)) {
        keyframeButton->setIcon(timelineWidget->keyframeYes);
    } else {
        keyframeButton->setIcon(timelineWidget->keyframeNo);
    }

    previousButton->setEnabled(property->hasBefore(scene->currentFrame));
    nextButton->setEnabled(property->hasAfter(scene->currentFrame));
}

void TimelinePropertyButton::propertyUpdated(PropertyBase *updatedProperty) {
    if (property != updatedProperty) {
        return;
    }
    updateKeyframe();
}

void TimelinePropertyButton::updateAnimating(PropertyBase *updatedProperty) {
    if (property != updatedProperty) {
        return;
    }

    previousButton->setVisible(property->isAnimating);
    keyframeButton->setVisible(property->isAnimating);
    nextButton->setVisible(property->isAnimating);
    keyframeRightSideSpacing->setVisible(!property->isAnimating);

    updateKeyframe();
}
