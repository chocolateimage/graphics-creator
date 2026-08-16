#pragma once
#include "element.hpp"

class VideoElement : public Element {
  public:
    VideoElement();
    virtual ~VideoElement() {}

    virtual AnimatableRender *createClass();

    Property<std::string> path{this, "path", ""};
    Property<int> scaleType{this, "scaleType", 2};

    virtual QString const typeName() { return "video"; }
};

class VideoElementRender : public ElementRender {
  public:
    VideoElementRender() : ElementRender() {}
    virtual ~VideoElementRender() {}

    PropertyRender<std::string> path{this};
    PropertyRender<int> scaleType{this};

    virtual bool render(uint32_t *target);
};
