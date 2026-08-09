#pragma once
#include "animatable/property.hpp"
#include "element.hpp"

class GroupElement : public Element {
  public:
    enum BoundsType { Children = 0, Scene = 1 };

    GroupElement();
    virtual ~GroupElement() {}

    AnimatableRender *createClass() override;
    AnimatableRender *toRender(const FrameInfo &frameInfo) override;
    QRect getRawBoundingBox(const FrameInfo &frameInfo) override;
    bool isResizable() const override { return false; }

    void ungroup();

    QString const typeName() override { return "group"; }

    Property<int> bounds{this, "bounds", 0};
};

class GroupElementRender : public ElementRender {
  public:
    GroupElementRender() : ElementRender() {}
    virtual ~GroupElementRender() {}

    QList<ElementRender *> children;

    int sceneWidth;
    int sceneHeight;

    void prepare() override;
    Rect getRenderBox() override;
    bool render(uint32_t *target) override;

    PropertyRender<int> bounds{this};
};
