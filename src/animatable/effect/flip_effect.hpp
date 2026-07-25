#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class FlipEffectRender : public EffectRender {
  public:
    ~FlipEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<bool> horizontal{this};
    PropertyRender<bool> vertical{this};
};

class FlipEffect : public Effect {
  public:
    FlipEffect();
    ~FlipEffect() {};
    QString effectName() override { return "flip"; };
    AnimatableRender *createClass() override { return new FlipEffectRender(); };

    Property<bool> horizontal{this, "horizontal", true};
    Property<bool> vertical{this, "vertical", true};
};
