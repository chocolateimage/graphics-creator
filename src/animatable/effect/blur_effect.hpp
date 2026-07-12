#pragma once
#include "effect.hpp"

class BlurEffectRender : public EffectRender {
  public:
    ~BlurEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
};

class BlurEffect : public Effect {
  public:
    ~BlurEffect() {}
    AnimatableRender *createClass() override;
};
