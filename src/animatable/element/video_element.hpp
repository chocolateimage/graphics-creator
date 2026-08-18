#pragma once
#include "element.hpp"

class VideoElement : public Element {
  public:
    VideoElement();
    virtual ~VideoElement() {}

    AnimatableRender *createClass() override;
    AnimatableRender *toRender(const FrameInfo &frameInfo) override;
    QJsonObject serialize() override;
    void deserialize(const QJsonObject &obj) override;

    Property<std::string> path{this, "path", ""};
    Property<int> scaleType{this, "scaleType", 2};

    int frameOffset{0};

    QString const typeName() override { return "video"; }
};

class VideoElementRender : public ElementRender {
  public:
    VideoElementRender() : ElementRender() {}
    virtual ~VideoElementRender() {}

    PropertyRender<std::string> path{this};
    PropertyRender<int> scaleType{this};

    double secondsOffset;

    virtual bool render(uint32_t *target);
};
