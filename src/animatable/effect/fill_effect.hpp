#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class FillEffectRender : public EffectRender {
  public:
    ~FillEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<Brush> fill{this};
};

class FillEffect : public Effect {
  public:
    FillEffect();
    ~FillEffect() {};
    QString effectName() override { return "fill"; };
    AnimatableRender *createClass() override { return new FillEffectRender(); };

    Property<Brush> fill{this, "fill", {}};
};
