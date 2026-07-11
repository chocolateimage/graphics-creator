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
    virtual ~PropertyBase() {
        for (auto keyframe : keyframes) {
            delete keyframe;
        }
    }
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
        if (!isAnimating || keyframes.size() == 1) {
            return ((Keyframe<T> *)keyframes[0])->value;
        }

        Keyframe<T> *start = (Keyframe<T> *)keyframes[0];
        Keyframe<T> *end = start;
        size_t index = 0;
        for (KeyframeBase *keyframe : keyframes) {
            if (frameInfo.frameIndex >= keyframe->frame) {
                start = (Keyframe<T> *)keyframe;
                if (index == keyframes.size() - 1) {
                    end = (Keyframe<T> *)keyframe;
                } else {
                    end = (Keyframe<T> *)keyframes[index + 1];
                }
            }
            index++;
        }

        if constexpr (std::is_arithmetic_v<T>) {
            float time = 0;

            if (end->frame != start->frame) {
                time = (float)(frameInfo.frameIndex - start->frame) /
                       (end->frame - start->frame);
            }

            T newValue = start->value + (end->value - start->value) * time;
            return newValue;
        } else {
            return start->value;
        }
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
            bool found = false;
            int insertIndex = keyframes.size();
            int index = 0;
            for (auto _keyframe : keyframes) {
                Keyframe<T> *keyframe = (Keyframe<T> *)_keyframe;
                if (keyframe->frame == frameInfo.frameIndex) {
                    keyframe->value = value;
                    found = true;
                } else if (keyframe->frame > frameInfo.frameIndex) {
                    insertIndex = index;
                    break;
                }
                index++;
            }
            if (!found) {
                keyframes.insert(keyframes.begin() + insertIndex,
                                 new Keyframe<T>(frameInfo.frameIndex, value));
            }
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
