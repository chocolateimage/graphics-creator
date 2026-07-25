#pragma once
#include "effect.hpp"

class PlaceOriginalEffectRender : public EffectRender {
  public:
    ~PlaceOriginalEffectRender() {}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;
};

class PlaceOriginalEffect : public Effect {
  public:
    PlaceOriginalEffect();
    ~PlaceOriginalEffect() {};
    QString effectName() override { return "placeOriginal"; };
    AnimatableRender *createClass() override {
        return new PlaceOriginalEffectRender();
    };
};
