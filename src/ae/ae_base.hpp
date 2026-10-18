#pragma once
#include <QDebug>
#include <cstdint>

struct AERect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

struct Fixed {
    uint16_t fractional;
    int16_t number;
};

inline double fixedToDouble(const Fixed &fixed) {
    return fixed.number + fixed.fractional / 65536.;
}

inline Fixed doubleToFixed(double value) {
    int val2 = value * 65536.;
    return *(Fixed *)&val2;
}

struct Scale {
    int32_t num;
    uint32_t den;
};

struct ARGB {
    uint8_t a, r, g, b;
};

struct PixelFloat {
    float a;
    float r;
    float g;
    float b;
};

struct RGBAf {
    float r, g, b, a;
};

struct Rectf {
    float x, y, w, h;
};

struct AEMemoryHandle {
    AEMemoryHandle();
    ~AEMemoryHandle();
    AEMemoryHandle *copy();

    void *data{nullptr};
    size_t size{0};
};

struct AEVersion {
    int16_t major;
    int16_t minor;
};

struct AETime {
    int32_t value;
    uint32_t scale;
};

struct AECompositeMode {
    enum TransferMode {
        None = -1,
        Copy,
        Behind,
        InFront,
        Dissolve,
        Add,
        Multiply,
        Screen,
        Overlay,
        SoftLight,
        HardLight,
        Darken,
        Lighten,
        Difference,
        Hue,
        Saturation,
        Color,
        Luminosity,
        MultiplyAlpha,
        MultiplyAlphaLuma,
        MultiplyNotAlpha,
        MultiplyNotAlphaLuma,
        AdditivePremultiplied,
        AlphaAdd,
        ColorDodge,
        ColorBurn,
        Exclusion,
        Difference2,
        ColorDodge2,
        ColorBurn2,
        LinearDodge,
        LinearBurn,
        LinearLight,
        VividLight,
        PinLight,
        HardMix,
        LighterColor,
        DarkerColor,
        Subtract,
        Divide,
    };
    TransferMode transferMode;
    int32_t seed;
    uint8_t opacity;
    bool rgbOnly;
    uint16_t opacity2;
};
