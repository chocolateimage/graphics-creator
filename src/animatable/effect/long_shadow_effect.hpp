#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class LongShadowEffectRender : public EffectRender {
  public:
    ~LongShadowEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<int> distance{this};
    PropertyRender<Brush> fill{this};
    PropertyRender<bool> shadowOnly{this};
};

class LongShadowEffect : public Effect {
  public:
    LongShadowEffect();
    ~LongShadowEffect() {};
    QString effectName() override { return "longShadow"; };
    AnimatableRender *createClass() override {
        return new LongShadowEffectRender();
    };

    Property<int> distance{this, "distance", 100};
    Property<Brush> fill{this, "fill", {}};
    Property<bool> shadowOnly{this, "shadowOnly", false};
};
