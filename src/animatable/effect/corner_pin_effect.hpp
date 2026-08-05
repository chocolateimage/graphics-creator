#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class CornerPinEffectRender : public EffectRender {
  public:
    ~CornerPinEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<Vector2DFloat> topLeft{this};
    PropertyRender<Vector2DFloat> topRight{this};
    PropertyRender<Vector2DFloat> bottomLeft{this};
    PropertyRender<Vector2DFloat> bottomRight{this};
};

class CornerPinEffect : public Effect {
  public:
    CornerPinEffect();
    ~CornerPinEffect() {};
    QString effectName() override { return "cornerPin"; };
    AnimatableRender *createClass() override {
        return new CornerPinEffectRender();
    };

    Property<Vector2DFloat> topLeft{this, "topLeft", {0, 0}};
    Property<Vector2DFloat> topRight{this, "topRight", {1, 0}};
    Property<Vector2DFloat> bottomLeft{this, "bottomLeft", {0, 1}};
    Property<Vector2DFloat> bottomRight{this, "bottomRight", {1, 1}};
};
