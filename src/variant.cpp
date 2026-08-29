#include "variant.hpp"
#include "math.hpp"
#include "render.hpp"
#include <QDebug>
#include <fontconfig/fontconfig.h>
#include <math.h>

Font Variant::defaultFont = {};

StrokeInfo TextSpan::strokeInfo() const {
    return {strokeWidth, strokeLineJoin};
}

Brush::Type getBrushTypeFromString(const std::string &str) {
    if (str == "singleColor") {
        return Brush::Type::SingleColor;
    } else if (str == "linearGradient") {
        return Brush::Type::LinearGradient;
    } else if (str == "radialGradient") {
        return Brush::Type::RadialGradient;
    }
    return Brush::Type::SingleColor;
}

Variant::Variant(VariantType variant) : m_variant(variant) {}

Variant::Variant(const Variant &variant) {
    m_variant = variant.m_variant;
    // TODO: deep copy structs.. maybe... is it necessary?
}

VariantType Variant::variant() const { return m_variant; }

VariantTypeEnum::Enum Variant::type() const {
    return (VariantTypeEnum::Enum)m_variant.index();
}

VariantTypeEnum::Enum Variant::typeFromString(const std::string &type) {
    if (type == "none") {
        return VariantTypeEnum::None;
    } else if (type == "string") {
        return VariantTypeEnum::String;
    } else if (type == "int") {
        return VariantTypeEnum::Int;
    } else if (type == "double") {
        return VariantTypeEnum::Double;
    } else if (type == "color") {
        return VariantTypeEnum::Color;
    } else if (type == "vector2dint") {
        return VariantTypeEnum::Vector2DInt;
    } else if (type == "font") {
        return VariantTypeEnum::Font;
    } else if (type == "bool") {
        return VariantTypeEnum::Bool;
    } else if (type == "easing") {
        return VariantTypeEnum::Easing;
    } else if (type == "brush") {
        return VariantTypeEnum::Brush;
    } else if (type == "textSpans") {
        return VariantTypeEnum::TextSpans;
    } else if (type == "vector2dfloat") {
        return VariantTypeEnum::Vector2DFloat;
    } else if (type == "elementSelection") {
        return VariantTypeEnum::ElementSelection;
    } else {
        return (VariantTypeEnum::Enum)-1;
    }
}

Variant Variant::getDefault(VariantTypeEnum::Enum type) {
    switch (type) {
    case VariantTypeEnum::None:
        return Variant(nullptr);
    case VariantTypeEnum::String:
        return Variant("");
    case VariantTypeEnum::Int:
        return Variant(0);
    case VariantTypeEnum::Double:
        return Variant(0.0);
    case VariantTypeEnum::Color:
        return Variant(Color{0, 0, 0, 255});
    case VariantTypeEnum::Vector2DInt:
        return Variant(Vector2DInt{0, 0});
    case VariantTypeEnum::Font:
        return Variant(defaultFont);
    case VariantTypeEnum::Bool:
        return Variant(false);
    case VariantTypeEnum::Easing:
        return Variant(Easing{""});
    case VariantTypeEnum::Brush:
        return Variant(Brush{});
    case VariantTypeEnum::TextSpans:
        return Variant(TextSpans{});
    case VariantTypeEnum::Vector2DFloat:
        return Variant(Vector2DFloat{0, 0});
    case VariantTypeEnum::ElementSelection:
        return Variant(ElementSelection{});
    }
    Q_UNREACHABLE();
}

bool Variant::isValidType(const std::string &type) {
    return (int)typeFromString(type) != -1;
}

std::function<double(double)> Easing::toFunction() {
    // Should I apologize? This is just really wrong.

#define IF_EASING_CURVE(name) else if (easingCurve == #name) return name;

    if (easingCurve == "linear") {
        return linear;
    }
    IF_EASING_CURVE(easeInQuad)
    IF_EASING_CURVE(easeOutQuad)
    IF_EASING_CURVE(easeInOutQuad)

    IF_EASING_CURVE(easeInCubic)
    IF_EASING_CURVE(easeOutCubic)
    IF_EASING_CURVE(easeInOutCubic)
    IF_EASING_CURVE(easeInQuart)

    IF_EASING_CURVE(easeOutQuart)
    IF_EASING_CURVE(easeInOutQuart)
    IF_EASING_CURVE(easeInQuint)
    IF_EASING_CURVE(easeOutQuint)

    IF_EASING_CURVE(easeInOutQuint)
    IF_EASING_CURVE(easeInSine)
    IF_EASING_CURVE(easeOutSine)
    IF_EASING_CURVE(easeInOutSine)

    IF_EASING_CURVE(easeInExpo)
    IF_EASING_CURVE(easeOutExpo)
    IF_EASING_CURVE(easeInOutExpo)
    IF_EASING_CURVE(easeInCirc)

    IF_EASING_CURVE(easeOutCirc)
    IF_EASING_CURVE(easeInOutCirc)
    IF_EASING_CURVE(easeInBack)
    IF_EASING_CURVE(easeOutBack)

    IF_EASING_CURVE(easeInOutBack)
    IF_EASING_CURVE(easeInElastic)
    IF_EASING_CURVE(easeOutElastic)
    IF_EASING_CURVE(easeInOutElastic)

    IF_EASING_CURVE(easeInBounce)
    IF_EASING_CURVE(easeOutBounce)
    IF_EASING_CURVE(easeInOutBounce)

    else {
        return linear;
    }
}

Font Font::fromPattern(const std::string &str) {
    FcPattern *pattern = FcNameParse((FcChar8 *)str.c_str());
    FcResult result;
    FcPattern *font = FcFontMatch(nullptr, pattern, &result);
    FcChar8 *rawFamily;
    FcChar8 *rawFileName;
    FcChar8 *rawStyle;
    int fontIndex;
    FcPatternGetString(font, FC_FILE, 0, &rawFileName);
    FcPatternGetString(font, FC_FAMILY, 0, &rawFamily);
    FcPatternGetString(font, FC_STYLE, 0, &rawStyle);
    FcPatternGetInteger(font, FC_INDEX, 0, &fontIndex);
    std::string family((char *)rawFamily);
    std::string fileName((char *)rawFileName);
    std::string style((char *)rawStyle);
    FcPatternDestroy(pattern);
    FcPatternDestroy(font);
    return Font{
        .path = fileName,
        .index = fontIndex,
        .displayName = family + " " + style,
        .pattern = str,
    };
}
