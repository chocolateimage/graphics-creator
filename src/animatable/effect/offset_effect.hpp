#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class OffsetEffectRender : public EffectRender {
  public:
    ~OffsetEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<int> x{this};
    PropertyRender<int> y{this};
};

class OffsetEffect : public Effect {
  public:
    OffsetEffect();
    ~OffsetEffect() {};
    QString effectName() override { return "offset"; };
    AnimatableRender *createClass() override {
        return new OffsetEffectRender();
    };

    Property<int> x{this, "x", 0};
    Property<int> y{this, "y", 0};
};
