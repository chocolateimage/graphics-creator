#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class LevelsEffectRender : public EffectRender {
  public:
    ~LevelsEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<int> fromBlack{this};
    PropertyRender<int> fromWhite{this};
    PropertyRender<double> gamma{this};
    PropertyRender<int> toBlack{this};
    PropertyRender<int> toWhite{this};
};

class LevelsEffect : public Effect {
  public:
    LevelsEffect();
    ~LevelsEffect() {};
    QString effectName() override { return "levels"; };
    AnimatableRender *createClass() override {
        return new LevelsEffectRender();
    };

    Property<int> fromBlack{this, "fromBlack", 0};
    Property<int> fromWhite{this, "fromWhite", 255};
    Property<double> gamma{this, "gamma", 1};
    Property<int> toBlack{this, "toBlack", 0};
    Property<int> toWhite{this, "toWhite", 255};
};
