#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class GainEffectRender : public EffectRender {
  public:
    ~GainEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<double> gain{this};
};

class GainEffect : public Effect {
  public:
    GainEffect();
    ~GainEffect() {};
    QString effectName() override { return "brightness"; };
    AnimatableRender *createClass() override { return new GainEffectRender(); };

    Property<double> gain{this, "brightness", 100};
};
