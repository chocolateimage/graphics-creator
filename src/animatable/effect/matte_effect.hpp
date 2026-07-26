#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class MatteEffectRender : public EffectRender {
  public:
    ~MatteEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<ElementSelection> selection{this};
    PropertyRender<bool> invert{this};
    PropertyRender<int> useForMatte{this};
};

class MatteEffect : public Effect {
  public:
    MatteEffect();
    ~MatteEffect() {};
    QString effectName() override { return "matte"; };
    AnimatableRender *createClass() override {
        return new MatteEffectRender();
    };

    Property<ElementSelection> selection{this, "selection", {}};
    Property<bool> invert{this, "invert", false};
    Property<int> useForMatte{this, "useForMatte", 0};
};
