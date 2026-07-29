#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class ExpandRenderBoxEffectRender : public EffectRender {
  public:
    ~ExpandRenderBoxEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<int> left{this};
    PropertyRender<int> top{this};
    PropertyRender<int> right{this};
    PropertyRender<int> bottom{this};
    PropertyRender<bool> showBox{this};
};

class ExpandRenderBoxEffect : public Effect {
  public:
    ExpandRenderBoxEffect();
    ~ExpandRenderBoxEffect() {};
    QString effectName() override { return "expandRenderBox"; };
    AnimatableRender *createClass() override {
        return new ExpandRenderBoxEffectRender();
    };

    Property<int> left{this, "left", 0};
    Property<int> top{this, "top", 0};
    Property<int> right{this, "right", 0};
    Property<int> bottom{this, "bottom", 0};
    Property<bool> showBox{this, "showBox", false};
};
