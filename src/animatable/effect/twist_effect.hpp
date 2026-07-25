#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class TwistEffectRender : public EffectRender {
  public:
    ~TwistEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<int> direction{this};
    PropertyRender<bool> flip{this};
};

class TwistEffect : public Effect {
  public:
    TwistEffect();
    ~TwistEffect() {};
    QString effectName() override { return "twist"; };
    AnimatableRender *createClass() override {
        return new TwistEffectRender();
    };

    Property<int> direction{this, "direction", 0};
    Property<bool> flip{this, "flip", true};
};
