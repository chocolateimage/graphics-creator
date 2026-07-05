#pragma once

class EffectRender : public AnimatableRender {};
class Effect : public Animatable {
  public:
    AnimatableRender *createClass() { return new EffectRender(); }
};
