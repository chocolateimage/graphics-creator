#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class PixelateEffectRender : public EffectRender {
  public:
    ~PixelateEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<int> strength{this};
};

class PixelateEffect : public Effect {
  public:
    PixelateEffect();
    ~PixelateEffect() {};
    QString effectName() override { return "pixelate"; };
    AnimatableRender *createClass() override {
        return new PixelateEffectRender();
    };

    Property<int> strength{this, "strength", 10};
};
