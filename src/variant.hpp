#pragma once

#include "lua.hpp"
#include <QList>
#include <algorithm>
#include <string>
#include <variant>

class Color {
  public:
    int r{255}, g{255}, b{255}, a{255};

    Color operator+(const Color &other) {
        return {std::min(r + other.r, 255), std::min(g + other.g, 255),
                std::min(b + other.b, 255), std::min(a + other.a, 255)};
    }
    Color operator-(const Color &other) {
        return {std::max(r - other.r, 0), std::max(g - other.g, 0),
                std::max(b - other.b, 0), std::max(a - other.a, 0)};
    }
    Color operator*(float value) {
        float clamped = std::clamp(value, 0.f, 1.f);
        return {(int)(r * clamped), (int)(g * clamped), (int)(b * clamped),
                (int)(a * clamped)};
    }
};

struct Vector2DInt {
    int x, y;
};

struct Font {
    std::string path;
    int index;
    std::string displayName;
};

struct Easing {
    std::string easingCurve;
};

struct Rect {
    int x, y, w, h;
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
};

class TextSpan {
  public:
    TextSpan() {}
    Font font;
    int fontSize{128};
    QString text{""};
    Brush fill{};
    bool newLine{false};

    int getLength() {
        // TODO: unicode handling (ICU?)
        // TODO: i hope people have enough RAM :)
        return 1; // text.length();
    };
};

class TextSpans {
  public:
    TextSpans() {}
    QList<TextSpan> spans;
};

Brush::Type getBrushTypeFromString(const std::string &str);

std::string getFontHash(const Font &font, int fontSize);

using VariantType =
    std::variant<std::monostate, std::string, int, double, Color, Vector2DInt,
                 Font, bool, Easing, Brush, TextSpans>;

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

    void pushLua(lua_State *L) const;

    static VariantTypeEnum::Enum typeFromString(const std::string &type);
    static Variant getDefault(VariantTypeEnum::Enum type);
    static Variant getFromLua(VariantTypeEnum::Enum type, lua_State *L,
                              int index);
    static bool isValidType(const std::string &type);

    static Font defaultFont;

  private:
    VariantType m_variant;
};
