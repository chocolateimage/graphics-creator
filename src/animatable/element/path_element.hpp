#pragma once
#include "element.hpp"
#include <QPainterPath>

class PathElement : public Element {
  public:
    PathElement();
    virtual ~PathElement() {}

    AnimatableRender *createClass() override;
    QRect getRawBoundingBox(const FrameInfo &frameInfo) override;
    bool isResizable() const override { return false; }

    Property<int> strokeWidth{this, "strokeWidth", 8};
    Property<Path> path{this, "path", {}};

    QString const typeName() override { return "path"; }
};

class PathElementRender : public ElementRender {
  public:
    PathElementRender() : ElementRender() {}
    virtual ~PathElementRender() {}

    PropertyRender<int> strokeWidth{this};
    PropertyRender<Path> path{this};

    QPainterPath painterPath;
    QRect rect;

    void prepare() override;
    bool render(uint32_t *target) override;
    Rect getRenderBox() override;
};
