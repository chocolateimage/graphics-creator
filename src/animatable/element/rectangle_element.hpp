#pragma once
#include "element.hpp"

class RectangleElement : public Element {
  public:
    RectangleElement();
    virtual ~RectangleElement() {}

    AnimatableRender *createClass() override;

    Property<Brush> fill{this, "fill", {}};
    Property<int> strokeWidth{this, "strokeWidth", 0};
    Property<Brush> stroke{this, "stroke", {}};
    Property<int> roundness{this, "roundness", 0};

    QString const typeName() override { return "rectangle"; }
};

class RectangleElementRender : public ElementRender {
  public:
    RectangleElementRender() : ElementRender() {};
    virtual ~RectangleElementRender() {}

    PropertyRender<Brush> fill{this};
    PropertyRender<int> strokeWidth{this};
    PropertyRender<Brush> stroke{this};
    PropertyRender<int> roundness{this};

    Rect getRenderBox() override;

    bool render(uint32_t *target) override;
};
