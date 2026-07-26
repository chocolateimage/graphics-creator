#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class TintEffectRender : public EffectRender {
  public:
    ~TintEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<Brush> tint{this};
    PropertyRender<double> amountToTint{this};
};

class TintEffect : public Effect {
  public:
    TintEffect();
    ~TintEffect() {};
    QString effectName() override { return "tint"; };
    AnimatableRender *createClass() override { return new TintEffectRender(); };

    Property<Brush> tint{this, "tint", {}};
    Property<double> amountToTint{this, "amountToTint", 100};
};
