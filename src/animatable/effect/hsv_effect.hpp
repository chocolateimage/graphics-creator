#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class HsvEffectRender : public EffectRender {
  public:
    ~HsvEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<int> hue{this};
    PropertyRender<int> saturation{this};
    PropertyRender<int> lightness{this};
};

class HsvEffect : public Effect {
  public:
    HsvEffect();
    ~HsvEffect() {};
    QString effectName() override { return "hsv"; };
    QString effectDescription() override;
    AnimatableRender *createClass() override { return new HsvEffectRender(); };

    Property<int> hue{this, "hue", 0};
    Property<int> saturation{this, "saturation", 0};
    Property<int> lightness{this, "lightness", 0};
};
