#pragma once
#include "animatable.hpp"
#include "frame_info.hpp"
#include "variant.hpp"
#include <QDebug>
#include <QEasingCurve>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
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

template <typename T> inline QJsonValue serializeAnyValue(const T &value) {
    return value;
}

template <> inline QJsonValue serializeAnyValue(const Color &value) {
    QJsonObject obj;
    if (value.r != 255)
        obj["r"] = value.r;
    if (value.g != 255)
        obj["g"] = value.g;
    if (value.b != 255)
        obj["b"] = value.b;
    if (value.a != 255)
        obj["a"] = value.a;
    return obj;
}

template <> inline QJsonValue serializeAnyValue(const Brush &value) {
    QJsonObject obj;
    if (value.brushType != Brush::Type::SingleColor) {
        obj["type"] = value.brushType;
    }
    if (value.angle != 0) {
        obj["angle"] = value.angle;
    }
    obj["color1"] = serializeAnyValue(value.color1);
    obj["color2"] = serializeAnyValue(value.color2);
    return obj;
}

template <> inline QJsonValue serializeAnyValue(const Vector2DInt &value) {
    QJsonObject obj;
    obj["x"] = value.x;
    obj["y"] = value.y;
    return obj;
}

template <> inline QJsonValue serializeAnyValue(const Rect &value) {
    QJsonObject obj;
    obj["x"] = value.x;
    obj["y"] = value.y;
    obj["w"] = value.w;
    obj["h"] = value.h;
    return obj;
}

template <> inline QJsonValue serializeAnyValue(const std::string &value) {
    return QString::fromStdString(value);
}

template <> inline QJsonValue serializeAnyValue(const Font &value) {
    QJsonObject obj;
    if (value.pattern.empty()) {
        obj["displayName"] = serializeAnyValue(value.displayName);
        obj["index"] = value.index;
        obj["path"] = serializeAnyValue(value.path);
    } else {
        obj["p"] = serializeAnyValue(value.pattern);
    }
    return obj;
}

template <> inline QJsonValue serializeAnyValue(const Easing &value) {
    QJsonObject obj;
    obj["easingCurve"] = serializeAnyValue(value.easingCurve);
    return obj;
}

template <> inline QJsonValue serializeAnyValue(const TextSpans &value) {
    QJsonObject obj;
    QJsonArray spansArray;
    QList<QString> patternList;
    int index = 0;
    for (const auto &span : value.spans) {
        const auto &lastSpan = value.spans[std::max(0, index - 1)];
        bool mustDefine = index == 0;

        QJsonObject spanObj;
        if (mustDefine || lastSpan.text != span.text) {
            spanObj["t"] = span.text;
        }
        if (mustDefine || lastSpan.stroke != span.stroke) {
            spanObj["st"] = serializeAnyValue(span.stroke);
        }
        if (span.strokeWidth != 0) {
            spanObj["sw"] = span.strokeWidth;
        }
        if (mustDefine || lastSpan.strokeLineJoin != span.strokeLineJoin) {
            spanObj["sl"] = (int)span.strokeLineJoin;
        }
        if (mustDefine || lastSpan.fill != span.fill) {
            spanObj["f"] = serializeAnyValue(span.fill);
        }
        if (span.font.pattern.empty()) {
            spanObj["fo"] = serializeAnyValue(span.font);
        } else {
            QString pattern = QString::fromStdString(span.font.pattern);
            if (!patternList.contains(pattern)) {
                patternList.append(pattern);
            }
            int patternIndex = patternList.indexOf(pattern);
            if (patternIndex != 0) {
                spanObj["fo"] = patternIndex;
            }
        }
        if (mustDefine || lastSpan.fontSize != span.fontSize) {
            spanObj["s"] = span.fontSize;
        }
        if (!span.antialiased) {
            spanObj["aa"] = span.antialiased;
        }
        if (span.newLine) {
            spanObj["nl"] = span.newLine;
        }
        spansArray.append(spanObj);
        index++;
    }
    obj["spans"] = spansArray;
    if (!patternList.isEmpty()) {
        QJsonArray patternArray;
        for (const auto &pattern : patternList) {
            QJsonObject fontObj;
            fontObj["p"] = pattern;
            patternArray.append(fontObj);
        }
        obj["p"] = patternArray;
    }
    return obj;
}

template <> inline QJsonValue serializeAnyValue(const Vector2DFloat &value) {
    QJsonArray array;
    array << value.x;
    array << value.y;
    return array;
}

template <> inline QJsonValue serializeAnyValue(const ElementSelection &value) {
    QJsonObject obj;
    obj["elementId"] = value.elementId;
    obj["frameType"] = value.frameType;
    return obj;
}

template <typename T> inline T deserializeAnyValue(const QJsonValue &value) {
    return value;
}

template <> inline int deserializeAnyValue(const QJsonValue &value) {
    return value.toInt();
}

template <> inline double deserializeAnyValue(const QJsonValue &value) {
    return value.toDouble();
}

template <> inline bool deserializeAnyValue(const QJsonValue &value) {
    return value.toBool();
}

template <> inline std::string deserializeAnyValue(const QJsonValue &value) {
    return value.toString().toStdString();
}

template <> inline Vector2DFloat deserializeAnyValue(const QJsonValue &value) {
    QJsonArray array = value.toArray();
    return {(float)array[0].toDouble(), (float)array[1].toDouble()};
}

template <> inline Color deserializeAnyValue(const QJsonValue &value) {
    QJsonObject obj = value.toObject();
    return Color{.r = obj["r"].toInt(255),
                 .g = obj["g"].toInt(255),
                 .b = obj["b"].toInt(255),
                 .a = obj["a"].toInt(255)};
}

template <> inline Brush deserializeAnyValue(const QJsonValue &value) {
    QJsonObject obj = value.toObject();
    return Brush{.brushType = (Brush::Type)obj["type"].toInt(0),
                 .color1 = deserializeAnyValue<Color>(obj["color1"]),
                 .color2 = deserializeAnyValue<Color>(obj["color2"]),
                 .angle = obj["angle"].toDouble(0)};
}

template <> inline Easing deserializeAnyValue(const QJsonValue &value) {
    return Easing{.easingCurve =
                      value.toObject()["easingCurve"].toString().toStdString()};
}

template <> inline Font deserializeAnyValue(const QJsonValue &value) {
    QJsonObject obj = value.toObject();
    if (obj.contains("p")) {
        std::string path = obj["p"].toString().toStdString();
        return Font::fromPattern(path);
    } else {
        return Font{.path = obj["path"].toString().toStdString(),
                    .index = obj["index"].toInt(),
                    .displayName = obj["displayName"].toString().toStdString(),
                    .pattern = ""};
    }
}

template <> inline TextSpans deserializeAnyValue(const QJsonValue &value) {
    TextSpans spans{};
    QList<Font> fonts;
    for (const auto &fontValue : value["p"].toArray()) {
        fonts.append(deserializeAnyValue<Font>(fontValue));
    }

    int index = 0;
    for (const auto &spanValue : value["spans"].toArray()) {
        QJsonObject spanObj = spanValue.toObject();
        TextSpan span{};
        TextSpan *lastSpan = index > 0 ? &spans.spans[index - 1] : &span;
        if (spanObj.contains("t")) {
            span.text = spanObj["t"].toString();
        } else {
            span.text = lastSpan->text;
        }
        if (spanObj.contains("st")) {
            span.stroke = deserializeAnyValue<Brush>(spanObj["st"]);
        } else {
            span.stroke = lastSpan->stroke;
        }
        span.strokeWidth = spanObj["sw"].toInt(0);
        if (spanObj.contains("sl")) {
            span.strokeLineJoin = (FT_Stroker_LineJoin)spanObj["sl"].toInt();
        } else {
            span.strokeLineJoin = lastSpan->strokeLineJoin;
        }
        if (spanObj.contains("f")) {
            span.fill = deserializeAnyValue<Brush>(spanObj["f"]);
        } else {
            span.fill = lastSpan->fill;
        }
        if (spanObj["fo"].isObject()) {
            span.font = deserializeAnyValue<Font>(spanObj["fo"]);
        } else {
            span.font = fonts[spanObj["fo"].toInt(0)];
        }
        if (spanObj.contains("s")) {
            span.fontSize = spanObj["s"].toInt();
        } else {
            span.fontSize = lastSpan->fontSize;
        }
        span.antialiased = spanObj["aa"].toBool(true);
        span.newLine = spanObj["nl"].toBool(false);
        spans.spans.append(span);
        index++;
    }
    return spans;
}

template <>
inline ElementSelection deserializeAnyValue(const QJsonValue &value) {
    QJsonObject obj = value.toObject();
    ElementSelection elementSelection;
    elementSelection.elementId = obj["elementId"].toString();
    elementSelection.frameType =
        (ElementSelection::FrameType)obj["frameType"].toInt();
    return elementSelection;
}

class KeyframeBase {
  public:
    KeyframeBase(PropertyBase *property, int frame)
        : property(property), frame(frame) {}
    PropertyBase *property;
    virtual ~KeyframeBase() {}
    int frame;
    QEasingCurve easing{QEasingCurve::Linear};

    virtual inline QJsonValue serializeValue() { return {}; }
};

template <typename T> class Keyframe : public KeyframeBase {
  public:
    Keyframe(PropertyBase *property, int frame, T value)
        : KeyframeBase(property, frame), value(value) {}
    virtual ~Keyframe() {}
    T value;

    virtual inline QJsonValue serializeValue() {
        return serializeAnyValue(value);
    }
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

    virtual QJsonObject serialize() { return {}; }
    virtual void deserialize(const QJsonObject &obj) {}

    // does set(get(frameInfo), frameInfo)
    virtual void addToPosition(const FrameInfo &frameInfo) {}

    bool isAnimating{false};
    QList<QString> flags;
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

    void updateBoundsToEnumList() {
        setMin(0);
        setMax(enumList.size() - 1);
    }

    virtual QJsonObject serialize() {
        QJsonObject obj;
        QJsonArray keyframesArray;
        for (auto keyframeBase : keyframes) {
            Keyframe<T> *keyframe = (Keyframe<T> *)keyframeBase;
            QJsonObject keyframeObject;
            int easingType = keyframe->easing.type();
            if (easingType != 0) {
                keyframeObject["easing"] = easingType;
            }
            keyframeObject["value"] = keyframe->serializeValue();
            if (keyframe->frame != 0) {
                keyframeObject["frame"] = keyframe->frame;
            }
            keyframesArray.append(keyframeObject);
        }
        obj["keyframes"] = keyframesArray;
        if (isAnimating) {
            obj["isAnimating"] = isAnimating;
        }
        return obj;
    }

    virtual void deserialize(const QJsonObject &obj) {
        isAnimating = obj["isAnimating"].toBool(false);
        for (auto keyframe : keyframes) {
            delete keyframe;
        }
        keyframes.clear();
        for (auto keyframeJson : obj["keyframes"].toArray()) {
            QJsonObject keyframeObj = keyframeJson.toObject();
            Keyframe<T> *keyframe =
                new Keyframe<T>(this, keyframeObj["frame"].toInt(0),
                                deserializeAnyValue<T>(keyframeObj["value"]));
            keyframe->easing = QEasingCurve(
                (QEasingCurve::Type)(keyframeObj["easing"].toInt(0)));
            keyframes.push_back(keyframe);
        }
    }

    T min;
    bool hasMin{false};
    T max;
    bool hasMax{false};

    // Only useful in Property<int>. When it's not empty, then a dropdown is
    // shown in the properties with the string values.
    std::vector<std::string> enumList;

    std::string suffix;
};

class PropertyRenderBase {
  public:
    PropertyRenderBase() {}
    virtual ~PropertyRenderBase() {}
    virtual void set(PropertyBase *property, const FrameInfo &frameInfo) {};
};

template <typename T> class PropertyRender : public PropertyRenderBase {
  public:
    constexpr operator T &() { return value; }

    PropertyRender(AnimatableRender *animatable) : PropertyRenderBase() {
        animatable->addProperty(this);
    };
    virtual ~PropertyRender() {}

    inline T &get() { return value; }
    virtual void set(PropertyBase *property, const FrameInfo &frameInfo) {
        auto propertyTyped = dynamic_cast<Property<T> *>(property);
        value = propertyTyped->get(frameInfo);
    }

    T value;
};
