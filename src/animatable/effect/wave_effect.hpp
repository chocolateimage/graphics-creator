#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class WaveEffectRender : public EffectRender {
  public:
    ~WaveEffectRender() {}

    PropertyRender<int> extend{this};
    PropertyRender<int> infrequency{this};
    PropertyRender<double> speed{this};

    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;
};

class WaveEffect : public Effect {
  public:
    WaveEffect();
    ~WaveEffect() {}
    QString effectName() override { return "wave"; };

    Property<int> extend{this, "extend", 100};
    Property<int> infrequency{this, "infrequency", 100};
    Property<double> speed{this, "speed", 5};

    AnimatableRender *createClass() override { return new WaveEffectRender(); }
};
