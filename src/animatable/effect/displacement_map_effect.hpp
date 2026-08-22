#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class DisplacementMapEffectRender : public EffectRender {
  public:
    ~DisplacementMapEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<ElementSelection> selection{this};
    PropertyRender<int> xDisplacement{this};
    PropertyRender<int> yDisplacement{this};
};

class DisplacementMapEffect : public Effect {
  public:
    DisplacementMapEffect();
    ~DisplacementMapEffect() {};
    QString effectName() override { return "displacementMap"; };
    AnimatableRender *createClass() override {
        return new DisplacementMapEffectRender();
    };

    Property<ElementSelection> selection{this, "selection", {}};
    Property<int> xDisplacement{this, "xDisplacement", 50};
    Property<int> yDisplacement{this, "yDisplacement", 50};
};
