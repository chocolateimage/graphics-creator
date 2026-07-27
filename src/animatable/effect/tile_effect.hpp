#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class TileEffectRender : public EffectRender {
  public:
    ~TileEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<double> multiplyX{this};
    PropertyRender<double> multiplyY{this};
    PropertyRender<bool> mirrorOdd{this};
    PropertyRender<bool> showOriginal{this};
    PropertyRender<double> shift{this};
    PropertyRender<int> shiftDirection{this};
};

class TileEffect : public Effect {
  public:
    TileEffect();
    ~TileEffect() {};
    QString effectName() override { return "tile"; };
    AnimatableRender *createClass() override { return new TileEffectRender(); };

    Property<double> multiplyX{this, "multiplyX", 1};
    Property<double> multiplyY{this, "multiplyY", 1};
    Property<bool> mirrorOdd{this, "mirrorOdd", false};
    Property<bool> showOriginal{this, "showOriginal", true};
    Property<double> shift{this, "shift", 0};
    Property<int> shiftDirection{this, "shiftDirection", 0};
};
