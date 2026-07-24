#pragma once
#include "element.hpp"

class ImageElement : public Element {
  public:
    ImageElement();
    virtual ~ImageElement() {}

    virtual AnimatableRender *createClass();

    Property<std::string> path{this, "path", ""};
    Property<bool> scaled{this, "scaled", true};

    virtual QString const typeName() { return "image"; }
};

class ImageElementRender : public ElementRender {
  public:
    ImageElementRender() : ElementRender() {}
    virtual ~ImageElementRender() {}

    PropertyRender<std::string> path{this};
    PropertyRender<bool> scaled{this};

    virtual bool render(uint32_t *target);
};
