#pragma once
#include "animatable.hpp"
#include "variant.hpp"
#include <QDebug>
#include <string>

class PropertyBase {
  public:
    PropertyBase(const std::string &name, Animatable *animatable)
        : animatable(animatable), name(name) {}
    virtual ~PropertyBase() {}
    virtual Variant toVariant() {
        qCritical() << "This is bad";
        return Variant{std::monostate{}};
    };
    Animatable *animatable;
    std::string name;
};

template <typename T> class Property : public PropertyBase {
  public:
    Property(Animatable *animatable, const std::string &name, T defaultValue)
        : PropertyBase(name, animatable), value(defaultValue) {
        animatable->addProperty(this);
    }
    virtual ~Property() {}

    // TODO: keyframes

    T get() { return value; }
    virtual Variant toVariant() { return Variant{value}; };
    void setConstant(T value) {
        this->value = value;
        animatable->_propertyUpdated(this);
    };

    T value;
};

class PropertyRenderBase {
  public:
    PropertyRenderBase() {}
    virtual ~PropertyRenderBase() {}
    virtual void set(PropertyBase *property) {};
};

template <typename T> class PropertyRender : public PropertyRenderBase {
  public:
    PropertyRender(AnimatableRender *animatable) : PropertyRenderBase() {
        animatable->addProperty(this);
    };
    virtual ~PropertyRender() {}

    T get() { return value; }
    virtual void set(PropertyBase *property) {
        auto propertyTyped = dynamic_cast<Property<T> *>(property);
        value = propertyTyped->get();
    }

    T value;
};
