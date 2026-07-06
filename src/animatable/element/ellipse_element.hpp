#pragma once
#include "element.hpp"

class EllipseElement : public Element {
  public:
    EllipseElement();
    virtual ~EllipseElement() {}

    virtual AnimatableRender *createClass();

    Property<Brush> fill{this, "fill", {}};
};

class EllipseElementRender : public ElementRender {
  public:
    EllipseElementRender() : ElementRender() {}
    virtual ~EllipseElementRender() {}

    PropertyRender<Brush> fill{this};

    virtual bool render(uint32_t *target);
};
