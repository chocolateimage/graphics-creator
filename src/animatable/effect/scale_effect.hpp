#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class ScaleEffectRender : public EffectRender {
  public:
    ~ScaleEffectRender() {}

    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<bool> bilinear{this};

    PropertyRender<double> scaleX{this};
    PropertyRender<double> scaleY{this};

    PropertyRender<double> alignX{this};
    PropertyRender<double> alignY{this};
};

class ScaleEffect : public Effect {
  public:
    ScaleEffect();
    ~ScaleEffect() {}
    QString effectName() override { return "scale"; };

    Property<bool> bilinear{this, "bilinear", true};

    Property<double> scaleX{this, "scaleX", 1};
    Property<double> scaleY{this, "scaleY", 1};

    Property<double> alignX{this, "alignX", 0.5};
    Property<double> alignY{this, "alignY", 0.5};

    AnimatableRender *createClass() override { return new ScaleEffectRender(); }
};
