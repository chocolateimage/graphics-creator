#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class BrightnessEffectRender : public EffectRender {
  public:
    ~BrightnessEffectRender() {}

    PropertyRender<int> brightness{this};

    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
};

class BrightnessEffect : public Effect {
  public:
    BrightnessEffect();
    ~BrightnessEffect() {};
    QString effectName() override { return "brightness2"; };
    AnimatableRender *createClass() override {
        return new BrightnessEffectRender();
    };

    Property<int> brightness{this, "brightness", 0};
};
