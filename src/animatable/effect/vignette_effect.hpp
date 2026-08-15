#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class VignetteEffectRender : public EffectRender {
  public:
    ~VignetteEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<int> from{this};
    PropertyRender<int> to{this};
    PropertyRender<int> mode{this};
};

class VignetteEffect : public Effect {
  public:
    VignetteEffect();
    ~VignetteEffect() {};
    QString effectName() override { return "vignette"; };
    AnimatableRender *createClass() override {
        return new VignetteEffectRender();
    };

    Property<int> from{this, "from", 50};
    Property<int> to{this, "to", 1500};
    Property<int> mode{this, "mode", 0};
};
