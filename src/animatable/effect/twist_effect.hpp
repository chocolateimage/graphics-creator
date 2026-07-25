#pragma once
#include "effect.hpp"

class TwistEffectRender : public EffectRender {
  public:
    ~TwistEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
};

class TwistEffect : public Effect {
  public:
    TwistEffect();
    ~TwistEffect() {};
    QString effectName() override { return "twist"; };
    AnimatableRender *createClass() override {
        return new TwistEffectRender();
    };
};
