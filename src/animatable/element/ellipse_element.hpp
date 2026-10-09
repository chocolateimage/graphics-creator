#pragma once
#include "element.hpp"

class EllipseElement : public Element {
  public:
    EllipseElement();
    virtual ~EllipseElement() {}

    AnimatableRender *createClass() override;

    Property<Brush> fill{this, "fill", {}};
    Property<int> strokeWidth{this, "strokeWidth", 0};
    Property<Brush> stroke{this, "stroke", {}};

    QString const typeName() override { return "ellipse"; }
};

class EllipseElementRender : public ElementRender {
  public:
    EllipseElementRender() : ElementRender() {}
    virtual ~EllipseElementRender() {}

    PropertyRender<Brush> fill{this};
    PropertyRender<int> strokeWidth{this};
    PropertyRender<Brush> stroke{this};

    Rect getRenderBox() override;

    bool render(uint32_t *target) override;
};
