#pragma once
#include "animatable.hpp"
#include <string>

class PropertyBase {
  public:
    PropertyBase(const std::string &name) : name(name) {}
    virtual ~PropertyBase() {}
    std::string name;
};

template <typename T> class Property : public PropertyBase {
  public:
    Property(Animatable *animatable, const std::string &name, T defaultValue)
        : PropertyBase(name), value(defaultValue) {
        animatable->addProperty(this);
    }
    virtual ~Property() {}

    T get() { return value; }
    void setConstant(T value) { this->value = value; };

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
