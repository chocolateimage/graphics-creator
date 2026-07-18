#pragma once
#include "effect.hpp"

class CrtEffectRender : public EffectRender {
  public:
    ~CrtEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
};

class CrtEffect : public Effect {
  public:
    CrtEffect();
    ~CrtEffect() {};
    QString effectName() override { return "MattiasCRT"; };
    AnimatableRender *createClass() override { return new CrtEffectRender(); };
};
