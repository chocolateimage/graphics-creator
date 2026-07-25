#pragma once
#include "element.hpp"

class RectangleElement : public Element {
  public:
    RectangleElement();
    virtual ~RectangleElement() {}

    virtual AnimatableRender *createClass();

    Property<Brush> fill{this, "fill", {}};
    Property<int> strokeWidth{this, "strokeWidth", 0};
    Property<Brush> stroke{this, "stroke", {}};
    Property<int> roundness{this, "roundness", 0};

    virtual QString const typeName() { return "rectangle"; }
};

class RectangleElementRender : public ElementRender {
  public:
    RectangleElementRender() : ElementRender() {};
    virtual ~RectangleElementRender() {}

    PropertyRender<Brush> fill{this};
    PropertyRender<int> strokeWidth{this};
    PropertyRender<Brush> stroke{this};
    PropertyRender<int> roundness{this};

    virtual Rect getRenderBox();

    virtual bool render(uint32_t *target);
};
