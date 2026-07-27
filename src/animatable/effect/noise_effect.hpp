#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class NoiseEffectRender : public EffectRender {
  public:
    ~NoiseEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<int> mode{this};
    PropertyRender<bool> perFrame{this};
};

class NoiseEffect : public Effect {
  public:
    NoiseEffect();
    ~NoiseEffect() {};
    QString effectName() override { return "noise"; };
    AnimatableRender *createClass() override {
        return new NoiseEffectRender();
    };

    Property<int> mode{this, "mode", 0};
    Property<bool> perFrame{this, "perFrame", true};
};
