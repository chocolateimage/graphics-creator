#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class DropShadowEffectRender : public EffectRender {
  public:
    ~DropShadowEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<double> distance{this};
    PropertyRender<double> angle{this};
    PropertyRender<Color> color{this};
    PropertyRender<int> softness{this};
    PropertyRender<bool> shadowOnly{this};
};

class DropShadowEffect : public Effect {
  public:
    DropShadowEffect();
    ~DropShadowEffect() {};
    QString effectName() override { return "dropShadow"; };
    AnimatableRender *createClass() override {
        return new DropShadowEffectRender();
    };

    Property<double> distance{this, "distance", 10};
    Property<double> angle{this, "angle", 135};
    Property<Color> color{this, "color", {0, 0, 0, 128}};
    Property<int> softness{this, "softness", 3};
    Property<bool> shadowOnly{this, "shadowOnly", false};
};
