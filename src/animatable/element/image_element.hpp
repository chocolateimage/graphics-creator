#pragma once
#include "element.hpp"

class ImageElement : public Element {
  public:
    ImageElement();
    virtual ~ImageElement() {}

    AnimatableRender *createClass() override;

    Property<std::string> path{this, "path", ""};
    Property<bool> scaled{this, "scaled", true};

    QString const typeName() override { return "image"; }
};

class ImageElementRender : public ElementRender {
  public:
    ImageElementRender() : ElementRender() {}
    virtual ~ImageElementRender() {}

    PropertyRender<std::string> path{this};
    PropertyRender<bool> scaled{this};

    bool render(uint32_t *target) override;
};
