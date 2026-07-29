#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class RotationEffectRender : public EffectRender {
  public:
    ~RotationEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<double> angle{this};
    PropertyRender<double> pivotX{this};
    PropertyRender<double> pivotY{this};
};

class RotationEffect : public Effect {
  public:
    RotationEffect();
    ~RotationEffect() {};
    QString effectName() override { return "rotation"; };
    AnimatableRender *createClass() override {
        return new RotationEffectRender();
    };

    Property<double> angle{this, "angle", 0};
    Property<double> pivotX{this, "pivotX", 50};
    Property<double> pivotY{this, "pivotY", 50};
};
