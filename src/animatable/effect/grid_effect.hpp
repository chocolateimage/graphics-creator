#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class GridEffectRender : public EffectRender {
  public:
    ~GridEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;

    PropertyRender<Color> lineColor{this};
    PropertyRender<Color> backgroundColor{this};
    PropertyRender<int> lineWidth{this};
    PropertyRender<int> lineHeight{this};
    PropertyRender<int> boxWidth{this};
    PropertyRender<int> boxHeight{this};
};

class GridEffect : public Effect {
  public:
    GridEffect();
    ~GridEffect() {}
    QString effectName() override { return "Grid"; };
    AnimatableRender *createClass() override { return new GridEffectRender(); };

    Property<Color> lineColor{this, "lineColor", {0, 0, 0, 255}};
    Property<Color> backgroundColor{
        this, "backgroundColor", {255, 255, 255, 255}};
    Property<int> lineWidth{this, "lineWidth", 3};
    Property<int> lineHeight{this, "lineHeight", 3};
    Property<int> boxWidth{this, "boxWidth", 10};
    Property<int> boxHeight{this, "boxHeight", 10};
};
