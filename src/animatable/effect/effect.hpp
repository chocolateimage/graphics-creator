#pragma once
#include "animatable/animatable.hpp"
#include "variant.hpp"

class ElementRender;
class RenderThread;

class EffectRender : public AnimatableRender {
  public:
    virtual Rect getRenderBox(const Rect &lastBox);
    Rect renderBox;
    ElementRender *element;
    RenderThread *renderThread;

    Rect originalBox;
    uint32_t *originalValues;

    int currentFrame{0};
    double currentSeconds{0};

    virtual bool render(const uint32_t *source, const Rect &sourceRect,
                        uint32_t *target) = 0;
};

class Effect : public Animatable {
    Q_OBJECT
  public:
    virtual ~Effect() {}

    bool collapsed{false};

    void setCollapsed(bool newValue);
    virtual QString effectName() = 0;
    virtual QString effectDescription() { return ""; };
    QJsonObject serialize() override;
    void deserialize(const QJsonObject &obj) override;

  signals:
    void collapsedChanged(bool newValue);
};
