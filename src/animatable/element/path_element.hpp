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

    Property<Brush> fill{this, "fill", Brush::fromColor(Color{0, 0, 0, 0})};
    Property<int> strokeWidth{this, "strokeWidth", 8};
    Property<Brush> stroke{this, "stroke", {}};
    Property<int> cap{this, "cap", 0};
    Property<int> join{this, "join", 0};
    Property<Path> path{this, "path", {}};

    QString const typeName() override { return "path"; }
};

class PathElementRender : public ElementRender {
  public:
    PathElementRender() : ElementRender() {}
    virtual ~PathElementRender() {}

    PropertyRender<Brush> fill{this};
    PropertyRender<int> strokeWidth{this};
    PropertyRender<Brush> stroke{this};
    PropertyRender<int> cap{this};
    PropertyRender<int> join{this};
    PropertyRender<Path> path{this};

    QPainterPath painterPath;
    QRect rect;

    void prepare() override;
    bool render(uint32_t *target) override;
    Rect getRenderBox() override;
};
