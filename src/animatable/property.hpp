#pragma once
#include "animatable.hpp"
#include "frame_info.hpp"
#include "variant.hpp"
#include <QDebug>
#include <QEasingCurve>
#include <QString>
#include <string>

template <class T, class = void> struct is_lerpable : std::false_type {};

template <class T>
struct is_lerpable<
    T, std::void_t<decltype(std::declval<T>() + std::declval<T>()),
                   decltype(std::declval<T>() - std::declval<T>()),
                   decltype(std::declval<T>() * std::declval<float>())>>
    : std::true_type {};

template <> struct is_lerpable<Brush> : std::true_type {};

template <typename T> inline T lerp(T a, T b, float value) {
    return a * (1.f - value) + (b * value);
}

template <> inline Brush lerp(Brush a, Brush b, float value) {
    return {
        .brushType = a.brushType,
        .color1 = lerp(a.color1, b.color1, value),
        .color2 = lerp(a.color2, b.color2, value),
        .angle = lerp(a.angle, b.angle, value),
    };
}

class KeyframeBase {
  public:
    KeyframeBase(PropertyBase *property, int frame)
        : property(property), frame(frame) {}
    PropertyBase *property;
    virtual ~KeyframeBase() {}
    int frame;
    QEasingCurve easing{QEasingCurve::Linear};
};

template <typename T> class Keyframe : public KeyframeBase {
  public:
    Keyframe(PropertyBase *property, int frame, T value)
        : KeyframeBase(property, frame), value(value) {}
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
    QString getDisplayName() {
        QString label;
        QString word;
        int len = name.size();
        for (int i = 0; i < len; i++) {
            char character = name[i];
            bool newWord = std::isupper(character);
            if (newWord) {
                label += " " + word;
                word = "";
            }

            if (i == 0) {
                character = std::toupper(character);
            }

            word += character;
        }
        label += " " + word;
        label = label.trimmed();
        return label;
    }
    std::vector<KeyframeBase *> keyframes;
    Animatable *animatable;
    std::string name;
    void toggleAnimating(const FrameInfo &frameInfo) {
        isAnimating = !isAnimating;
        if (isAnimating) {
            keyframes[0]->frame = frameInfo.frameIndex;
        } else {
            while (keyframes.size() > 1) {
                delete keyframes.back();
                keyframes.pop_back();
            }
        }
        animatable->_propertyIsAnimatingUpdated(this);
        if (!isAnimating) {
            animatable->_propertyUpdated(this);
        }
    }

    bool has(int frame) {
        if (!isAnimating)
            return false;

        for (auto keyframe : keyframes) {
            if (keyframe->frame == frame)
                return true;
        }

        return false;
    }

    bool hasBefore(int frame) {
        if (!isAnimating)
            return false;

        for (auto keyframe : keyframes) {
            if (keyframe->frame < frame)
                return true;
        }

        return false;
    }

    bool hasAfter(int frame) {
        if (!isAnimating)
            return false;

        for (auto keyframe : keyframes) {
            if (keyframe->frame > frame)
                return true;
        }

        return false;
    }

    bool remove(int frame) {
        if (!isAnimating)
            return false;

        for (auto it = keyframes.begin(); it != keyframes.end(); it++) {
            if ((*it)->frame != frame) {
                continue;
            }

            if (keyframes.size() == 1) {
                toggleAnimating({frame});
            } else {
                delete *it;
                keyframes.erase(it);
                animatable->_propertyUpdated(this);
            }
            return true;
        }
        return false;
    }

    bool move(int from, int to) {
        if (!isAnimating)
            return false;

        if (from == to)
            return false;

        if (to < 0)
            return false;

        KeyframeBase *fromKeyframe{nullptr};
        for (auto keyframe : keyframes) {
            if (keyframe->frame == to) {
                return false;
            }
            if (keyframe->frame == from) {
                fromKeyframe = keyframe;
            }
        }

        if (!fromKeyframe)
            return false;

        fromKeyframe->frame = to;

        std::sort(keyframes.begin(), keyframes.end(),
                  [](KeyframeBase *a, KeyframeBase *b) {
                      return a->frame < b->frame;
                  });

        animatable->_propertyUpdated(this);
        return true;
    }

    // does set(get(frameInfo), frameInfo)
    virtual void addToPosition(const FrameInfo &frameInfo) {}

    bool isAnimating{false};
};

template <typename T> class Property : public PropertyBase {
  public:
    Property(Animatable *animatable, const std::string &name, T defaultValue)
        : PropertyBase(name, animatable) {
        keyframes.push_back(new Keyframe<T>(this, 0, defaultValue));
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

        if constexpr (is_lerpable<T>::value) {
            float time = 0;

            if (end->frame != start->frame) {
                time = (float)(frameInfo.frameIndex - start->frame) /
                       (end->frame - start->frame);

                if (time == 1) {
                    return end->value;
                }

                time = start->easing.valueForProgress(time);
            }

            if (time == 0) {
                return start->value;
            }

            T newValue = lerp(start->value, end->value, time);
            if constexpr (std::is_arithmetic_v<T>) {
                if (hasMin) {
                    if (newValue < min) {
                        newValue = min;
                    }
                }
                if (hasMax) {
                    if (newValue > max) {
                        newValue = max;
                    }
                }
            }
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
                keyframes.insert(
                    keyframes.begin() + insertIndex,
                    new Keyframe<T>(this, frameInfo.frameIndex, value));
            }
        } else {
            ((Keyframe<T> *)keyframes[0])->value = value;
        }
        animatable->_propertyUpdated(this);
    };

    virtual void addToPosition(const FrameInfo &frameInfo) {
        set(get(frameInfo), frameInfo);
    }

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

    inline T get() { return value; }
    virtual void set(PropertyBase *property, const FrameInfo &frameInfo) {
        auto propertyTyped = dynamic_cast<Property<T> *>(property);
        value = propertyTyped->get(frameInfo);
    }

    T value;
};
