#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class ThresholdEffectRender : public EffectRender {
  public:
    ~ThresholdEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<int> threshold{this};
    PropertyRender<int> channel{this};
};

class ThresholdEffect : public Effect {
  public:
    ThresholdEffect();
    ~ThresholdEffect() {};
    QString effectName() override { return "threshold"; };
    AnimatableRender *createClass() override {
        return new ThresholdEffectRender();
    };

    Property<int> threshold{this, "threshold", 128};
    Property<int> channel{this, "channel", 0};
};
