#pragma once
#include "animatable.hpp"
#include "frame_info.hpp"
#include "variant.hpp"
#include <QDebug>
#include <string>

class KeyframeBase {
  public:
    KeyframeBase(int frame) : frame(frame) {}
    virtual ~KeyframeBase() {}
    int frame;
    // TODO: easing/curve/whatever
};

template <typename T> class Keyframe : public KeyframeBase {
  public:
    Keyframe(int frame, T value) : KeyframeBase(frame), value(value) {}
    virtual ~Keyframe() {}
    T value;
};

class PropertyBase {
  public:
    PropertyBase(const std::string &name, Animatable *animatable)
        : animatable(animatable), name(name) {}
    virtual ~PropertyBase() {}
    virtual Variant toVariant(const FrameInfo &frameInfo) {
        qCritical() << "This is bad";
        return Variant{std::monostate{}};
    };
    virtual bool isAnimatable() { return true; }
    std::string getDisplayName() { return name; }
    std::vector<KeyframeBase *> keyframes;
    Animatable *animatable;
    std::string name;
    void toggleAnimating() {
        isAnimating = !isAnimating;
        animatable->_propertyIsAnimatingUpdated(this);
        if (!isAnimating) {
            animatable->_propertyUpdated(this);
        }
    }
    bool isAnimating{false};
};

template <typename T> class Property : public PropertyBase {
  public:
    Property(Animatable *animatable, const std::string &name, T defaultValue)
        : PropertyBase(name, animatable) {
        keyframes.push_back(new Keyframe<T>(0, defaultValue));
        animatable->addProperty(this);
    }
    virtual ~Property() {}

    T get(const FrameInfo &frameInfo) {
        // TODO: interpolate
        return ((Keyframe<T> *)keyframes[0])->value;
    }
    virtual Variant toVariant(const FrameInfo &frameInfo) {
        return Variant{get(frameInfo)};
    };
    void set(T value, const FrameInfo &frameInfo) {
        if constexpr (std::is_arithmetic_v<T>) {
            if (hasMin) {
                if (value < min) {
                    value = min;
                }
            }
            if (hasMax) {
                if (value > max) {
                    value = max;
                }
            }
        }

        if (isAnimating) {
            // TODO set/insert at right position
        } else {
            ((Keyframe<T> *)keyframes[0])->value = value;
        }
        animatable->_propertyUpdated(this);
    };

    void setMin(T value) {
        hasMin = true;
        min = value;
    }

    void setMax(T value) {
        hasMax = true;
        max = value;
    }

    T min;
    bool hasMin{false};
    T max;
    bool hasMax{false};
};

class PropertyRenderBase {
  public:
    PropertyRenderBase() {}
    virtual ~PropertyRenderBase() {}
    virtual void set(PropertyBase *property, const FrameInfo &frameInfo) {};
};

template <typename T> class PropertyRender : public PropertyRenderBase {
  public:
    constexpr operator T() { return value; }

    PropertyRender(AnimatableRender *animatable) : PropertyRenderBase() {
        animatable->addProperty(this);
    };
    virtual ~PropertyRender() {}

    T get() { return value; }
    virtual void set(PropertyBase *property, const FrameInfo &frameInfo) {
        auto propertyTyped = dynamic_cast<Property<T> *>(property);
        value = propertyTyped->get(frameInfo);
    }

    T value;
};
