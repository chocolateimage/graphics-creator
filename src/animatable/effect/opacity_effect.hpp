#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class OpacityEffectRender : public EffectRender {
  public:
    ~OpacityEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<double> opacity{this};
};

class OpacityEffect : public Effect {
  public:
    OpacityEffect();
    ~OpacityEffect() {};
    QString effectName() override { return "opacity"; };
    AnimatableRender *createClass() override {
        return new OpacityEffectRender();
    };

    Property<double> opacity{this, "opacity", 100};
};
