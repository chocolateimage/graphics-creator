#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class ColorLinkEffectRender : public EffectRender {
  public:
    ~ColorLinkEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<ElementSelection> selection{this};
};

class ColorLinkEffect : public Effect {
  public:
    ColorLinkEffect();
    ~ColorLinkEffect() {};
    QString effectName() override { return "colorLink"; };
    AnimatableRender *createClass() override {
        return new ColorLinkEffectRender();
    };

    Property<ElementSelection> selection{this, "selection", {}};
};
