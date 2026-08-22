#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"

class CropEffectRender : public EffectRender {
  public:
    ~CropEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;

    PropertyRender<int> left{this};
    PropertyRender<int> top{this};
    PropertyRender<int> right{this};
    PropertyRender<int> bottom{this};
};

class CropEffect : public Effect {
  public:
    CropEffect();
    ~CropEffect() {};
    QString effectName() override { return "crop"; };
    AnimatableRender *createClass() override { return new CropEffectRender(); };

    Property<int> left{this, "left", 0};
    Property<int> top{this, "top", 0};
    Property<int> right{this, "right", 0};
    Property<int> bottom{this, "bottom", 0};
};
