#pragma once
#include "element.hpp"

class RectangleElement : public Element {
  public:
    RectangleElement();
    virtual ~RectangleElement() {}

    virtual AnimatableRender *createClass();

    Property<Brush> fill{this, "fill", {}};
    Property<int> strokeWidth{this, "strokeWidth", 16};
};

class RectangleElementRender : public ElementRender {
  public:
    RectangleElementRender() : ElementRender() {};
    virtual ~RectangleElementRender() {}

    PropertyRender<Brush> fill{this};
    PropertyRender<int> strokeWidth{this};

    virtual Rect getRenderBox();

    virtual bool render(uint32_t *target);
};
