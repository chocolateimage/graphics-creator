#pragma once
#include "animatable/animatable.hpp"
#include "animatable/effect.hpp"
#include "animatable/property.hpp"
#include "variant.hpp"
#include <QObject>
#include <string>

static constexpr uint32_t makePixel(Color color) {
    return (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
}

struct Rect {
    int x, y, w, h;
};

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
    std::vector<Effect *> effects;
};

class ElementRender : public AnimatableRender {
    Q_OBJECT
  public:
    ElementRender() {};
    ~ElementRender() {};

    PropertyRender<int> x{this};
    PropertyRender<int> y{this};
    PropertyRender<int> w{this};
    PropertyRender<int> h{this};

    virtual Rect getRenderBox();

    virtual bool render(uint32_t *target) = 0;
};
