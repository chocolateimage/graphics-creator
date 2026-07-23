#pragma once
#include "effect.hpp"

class InvertEffectRender : public EffectRender {
  public:
    ~InvertEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
};

class InvertEffect : public Effect {
  public:
    ~InvertEffect() {}
    QString effectName() override { return "Invert"; }
    AnimatableRender *createClass() override {
        return new InvertEffectRender();
    }
};
