#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class InOutEffectRender : public EffectRender {
  public:
    ~InOutEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<double> in{this};
    PropertyRender<double> out{this};
    PropertyRender<bool> invert{this};
};

class InOutEffect : public Effect {
  public:
    InOutEffect();
    ~InOutEffect() {};
    QString effectName() override { return "inOut"; };
    AnimatableRender *createClass() override {
        return new InOutEffectRender();
    };

    Property<double> in{this, "in", 0};
    Property<double> out{this, "out", 5};
    Property<bool> invert{this, "invert", false};
};
