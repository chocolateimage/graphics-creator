#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class BrightnessEffectRender : public EffectRender {
  public:
    ~BrightnessEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<double> brightness{this};
};

class BrightnessEffect : public Effect {
  public:
    BrightnessEffect();
    ~BrightnessEffect() {};
    QString effectName() override { return "brightness"; };
    AnimatableRender *createClass() override {
        return new BrightnessEffectRender();
    };

    Property<double> brightness{this, "brightness", 100};
};
