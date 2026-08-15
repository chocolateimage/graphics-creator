#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class BlurEffectRender : public EffectRender {
  public:
    ~BlurEffectRender() {}

    PropertyRender<int> radius{this};
    PropertyRender<int> iterations{this};

    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;
};

class BlurEffect : public Effect {
  public:
    BlurEffect();
    ~BlurEffect() {}
    QString effectName() override { return "boxBlur"; };

    Property<int> radius{this, "radius", 32};
    Property<int> iterations{this, "iterations", 1};

    AnimatableRender *createClass() override;
};
