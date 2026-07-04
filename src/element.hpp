#pragma once
#include "variant.hpp"
#include <QObject>
#include <string>
#include <vector>

static constexpr uint32_t makePixel(Color color) {
    return (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
}

struct Rect {
    int x, y, w, h;
};

class Animatable;
class AnimatableRender;

class PropertyBase {
  public:
    PropertyBase(const std::string &name);
    virtual ~PropertyBase() {}
    std::string name;
};

template <typename T> class Property : public PropertyBase {
  public:
    Property(Animatable *animatable, const std::string &name, T defaultValue);
    virtual ~Property() {}

    T get();
    void setConstant(T value) { this->value = value; };

    T value;
};

class PropertyRenderBase {
  public:
    PropertyRenderBase();
    virtual ~PropertyRenderBase() {}
    virtual void set(PropertyBase *property) {};
};

template <typename T> class PropertyRender : public PropertyRenderBase {
  public:
    PropertyRender(AnimatableRender *animatable);
    virtual ~PropertyRender() {}

    T get();
    virtual void set(PropertyBase *property);

    T value;
};

class Animatable : public QObject {
    Q_OBJECT
  public:
    void addProperty(PropertyBase *property);
    std::vector<PropertyBase *> properties;

    virtual AnimatableRender *createClass() = 0;
    AnimatableRender *toRender();
};

class AnimatableRender : public QObject {
    Q_OBJECT
  public:
    void addProperty(PropertyRenderBase *property);
    std::vector<PropertyRenderBase *> properties;
};

class EffectRender : public AnimatableRender {};
class Effect : public Animatable {
  public:
    AnimatableRender *createClass() { return new EffectRender(); }
};

class Element : public Animatable {
    Q_OBJECT
  public:
    Element() {};
    ~Element();
    Property<int> x{this, "x", 0};
    Property<int> y{this, "y", 0};
    Property<int> w{this, "w", 100};
    Property<int> h{this, "h", 100};
    std::string name;
    std::vector<Effect *> effects;

    // TODO: "properties" that are named with strings so you can animate them
    // ("x", "y", "fill", whatever)
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

class RectangleElement : public Element {
  public:
    RectangleElement();
    virtual ~RectangleElement() {}

    virtual AnimatableRender *createClass();

    Property<Brush> fill{this, "fill", {}};
    Property<int> strokeWidth{this, "strokeWidth", 16};
};

class RectangleElementRender : public ElementRender {
  public:
    RectangleElementRender() : ElementRender() {};
    virtual ~RectangleElementRender() {}

    PropertyRender<Brush> fill{this};
    PropertyRender<int> strokeWidth{this};

    virtual Rect getRenderBox();

    virtual bool render(uint32_t *target);
};
