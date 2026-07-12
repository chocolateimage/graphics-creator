#pragma once
#include "animatable/animatable.hpp"
#include "variant.hpp"

class ElementRender;

class EffectRender : public AnimatableRender {
  public:
    virtual Rect getRenderBox();
    ElementRender *element;

    virtual bool render(const uint32_t *source, const Rect &sourceRect,
                        uint32_t *target) = 0;
};

class Effect : public Animatable {
  public:
    virtual ~Effect() {}
};
