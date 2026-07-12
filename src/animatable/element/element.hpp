#pragma once
#include "animatable/animatable.hpp"
#include "animatable/effect/effect.hpp"
#include "animatable/property.hpp"
#include "variant.hpp"
#include <QObject>
#include <string>

static constexpr uint32_t makePixel(Color color) {
    return (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
}

class Element : public Animatable {
    Q_OBJECT
  public:
    Element() {
        w.setMin(1);
        h.setMin(1);
    };
    ~Element();
    Property<int> x{this, "x", 0};
    Property<int> y{this, "y", 0};
    Property<int> w{this, "w", 100};
    Property<int> h{this, "h", 100};
    bool collapsed{true};
    std::vector<Effect *> effects;

    AnimatableRender *toRender(const FrameInfo &frameInfo) override;
    void addEffect(Effect *effect);
    void insertEffect(Effect *effect, int index);
    void removeEffect(Effect *effect);

  signals:
    void effectAdded(Effect *effect, int index);
    void effectRemoved(Effect *effect);
    void effectListUpdated();
    void effectPropertyUpdated(Effect *effect, PropertyBase *property);
};

class ElementRender : public AnimatableRender {
    Q_OBJECT
  public:
    ElementRender() {};
    ~ElementRender();

    PropertyRender<int> x{this};
    PropertyRender<int> y{this};
    PropertyRender<int> w{this};
    PropertyRender<int> h{this};
    std::vector<EffectRender *> effects;

    virtual Rect getRenderBox();

    virtual bool render(uint32_t *target) = 0;
};
