#pragma once

#include <QEasingCurve>
#include <QList>
#include <QRect>
#include <algorithm>
#include <freetype/ftstroke.h>
#include <string>
#include <variant>

class StrokeInfo;

struct Color {
    int r{255}, g{255}, b{255}, a{255};

    Color operator+(const Color &other) const {
        return {std::min(r + other.r, 255), std::min(g + other.g, 255),
                std::min(b + other.b, 255), std::min(a + other.a, 255)};
    }
    Color operator-(const Color &other) const {
        return {std::max(r - other.r, 0), std::max(g - other.g, 0),
                std::max(b - other.b, 0), std::max(a - other.a, 0)};
    }
    Color operator*(float value) const {
        float clamped = std::clamp(value, 0.f, 1.f);
        return {(int)(r * clamped), (int)(g * clamped), (int)(b * clamped),
                (int)(a * clamped)};
    }

    bool operator==(const Color &other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    bool operator!=(const Color &other) const { return !operator==(other); }
};

struct Vector2DInt {
    int x, y;
};

struct Vector2DFloat {
    Vector2DFloat() : x(0), y(0) {}
    Vector2DFloat(float value) : x(value), y(value) {}
    Vector2DFloat(float x, float y) : x(x), y(y) {}

    float x, y;

    Vector2DFloat operator+(const Vector2DFloat &other) const {
        return {x + other.x, y + other.y};
    }
    Vector2DFloat operator-(const Vector2DFloat &other) const {
        return {x - other.x, y - other.y};
    }
    Vector2DFloat operator*(const Vector2DFloat &other) const {
        return {x * other.x, y * other.y};
    }
    Vector2DFloat operator*(float value) const {
        return {x * value, y * value};
    }
};

struct Font {
    std::string path;
    int index;
    std::string displayName;
    std::string pattern;

    static Font fromPattern(const std::string &str);
};

struct Easing {
    std::string easingCurve;

    std::function<double(double)> toFunction();
};

struct Rect {
    int x, y, w, h;

    Rect united(const Rect &other) const {
        int left = std::min(x, other.x);
        int top = std::min(y, other.y);
        int right = std::max(x + w, other.x + other.w);
        int bottom = std::max(y + h, other.y + other.h);
        return {left, top, right - left, bottom - top};
    }

    QRect toQRect() const { return {x, y, w, h}; }

    static Rect fromQRect(const QRect &qrect) {
        return {qrect.x(), qrect.y(), qrect.width(), qrect.height()};
    }
};

struct Brush {
    enum Type {
        SingleColor,
        LinearGradient,
        RadialGradient,
    };

    Brush::Type brushType = Brush::Type::SingleColor;
    Color color1;
    Color color2;
    double angle{0};

    bool operator==(const Brush &other) const {
        return brushType == other.brushType && color1 == other.color1 &&
               color2 == other.color2 && angle == other.angle;
    }
    bool operator!=(const Brush &other) const { return !operator==(other); }
};

class TextSpan {
  public:
    TextSpan() {}

    Font font;
    int fontSize{128};
    // Only allowed to contain one character else weird things can happen.
    // I wanted to do multiple characters in one span but whatever :')
    QString text{""};
    Brush fill{};
    bool newLine{false};
    bool antialiased{true};

    double strokeWidth{0};
    Brush stroke{};
    FT_Stroker_LineJoin strokeLineJoin{FT_STROKER_LINEJOIN_ROUND};

    StrokeInfo strokeInfo() const;
};

class TextSpans {
  public:
    TextSpans() {}
    QList<TextSpan> spans;
};

class ElementSelection {
  public:
    enum FrameType {
        Source,
        Final,
    };

    ElementSelection() {}
    QString elementId;
    FrameType frameType{Final};
};

Brush::Type getBrushTypeFromString(const std::string &str);

using VariantType = std::variant<std::monostate, std::string, int, double,
                                 Color, Vector2DInt, Font, bool, Easing, Brush,
                                 TextSpans, Vector2DFloat, ElementSelection>;

struct VariantTypeEnum {
    enum Enum {
        None,
        String,
        Int,
        Double,
        Color,
        Vector2DInt,
        Font,
        Bool,
        Easing,
        Brush,
        TextSpans,
        Vector2DFloat,
        ElementSelection
    };
};

class Variant {
  public:
    Variant(VariantType variant);
    Variant(const Variant &variant);
    Variant &operator=(const Variant &) = default;

    VariantType variant() const;
    VariantTypeEnum::Enum type() const;

    template <typename T> T get() const { return std::get<T>(m_variant); }

    static VariantTypeEnum::Enum typeFromString(const std::string &type);
    static Variant getDefault(VariantTypeEnum::Enum type);
    static bool isValidType(const std::string &type);

    static Font defaultFont;

  private:
    VariantType m_variant;
};
