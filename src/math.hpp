#pragma once
#include <array>
#include <cmath>
#include <cstdint>

inline constexpr uint32_t makePixel(uint8_t red, uint8_t green, uint8_t blue,
                                    uint8_t alpha) {
    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

inline constexpr int pixelIndex(int x, int y, int stride) {
    return y * stride + x;
}

static inline float mix(float t, float min, float max) {
    return (t * (max - min)) + min;
}

static double saturate(double x) { return std::max(std::min(x, 1.), 0.); }

static double length(double x, double y) { return std::sqrt(x * x + y * y); }

static double distance(double x1, double y1, double x2, double y2) {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

static double linearstep(double from, double to, double x) {
    return saturate((x - from) / (to - from));
}

static double linear(double x) { return x; }

static double easeInQuad(double x) { return x * x; }

static double easeOutQuad(double x) { return 1 - (1 - x) * (1 - x); }

static double easeInOutQuad(double x) {
    if (x < 0.5) {
        return 2 * x * x;
    } else {
        return 1 - std::pow((-2 * x + 2), 2) / 2;
    }
}

static double easeInCubic(double x) { return x * x * x; }

static double easeOutCubic(double x) { return 1 - std::pow((1 - x), 3); }

static double easeInOutCubic(double x) {
    if (x < 0.5) {
        return 4 * x * x * x;
    } else {
        return 1 - std::pow((-2 * x + 2), 3) / 2;
    }
}

static double easeInQuart(double x) { return x * x * x * x; }

static double easeOutQuart(double x) { return 1 - std::pow((1 - x), 4); }

static double easeInOutQuart(double x) {
    if (x < 0.5) {
        return 8 * x * x * x * x;
    } else {
        return 1 - std::pow((-2 * x + 2), 4) / 2;
    }
}

static double easeInQuint(double x) { return x * x * x * x * x; }

static double easeOutQuint(double x) { return 1 - std::pow((1 - x), 5); }

static double easeInOutQuint(double x) {
    if (x < 0.5) {
        return 16 * x * x * x * x * x;
    } else {
        return 1 - std::pow((-2 * x + 2), 5) / 2;
    }
}

static double easeInSine(double x) { return 1 - std::cos((x * M_PI) / 2); }

static double easeOutSine(double x) { return std::sin((x * M_PI) / 2); }

static double easeInOutSine(double x) { return -(std::cos(M_PI * x) - 1) / 2; }

static double easeInExpo(double x) {
    if (x == 0) {
        return x;
    }
    return std::pow(2, (10 * x - 10));
}

static double easeOutExpo(double x) {
    if (x == 1) {
        return x;
    }
    return 1 - std::pow(2, (-10 * x));
}

static double easeInOutExpo(double x) {
    if (x == 0 || x == 1) {
        return x;
    }

    if (x < 0.5) {
        return std::pow(2, (20 * x - 10)) / 2;
    } else {
        return (2 - std::pow(2, (-20 * x + 10))) / 2;
    }
}

static double easeInCirc(double x) { return 1 - sqrt(1 - std::pow(x, 2)); }

static double easeOutCirc(double x) { return sqrt(1 - std::pow((x - 1), 2)); }

static double easeInOutCirc(double x) {
    if (x < 0.5) {
        return (1 - sqrt(1 - std::pow((2 * x), 2))) / 2;
    } else {
        return (sqrt(1 - std::pow((-2 * x + 2), 2)) + 1) / 2;
    }
}

static double easeInBack(double x) {
    return 2.70158 * x * x * x - 1.70158 * x * x;
}

static double easeOutBack(double x) {
    return 1 + 2.70158 * std::pow((x - 1), 3) + 1.70158 * std::pow((x - 1), 2);
}

static double easeInOutBack(double x) {
    if (x < 0.5) {
        return (std::pow((2 * x), 2) * ((2.5949095 + 1) * 2 * x - 2.5949095)) /
               2;
    } else {
        return (std::pow((2 * x - 2), 2) *
                    ((2.5949095 + 1) * (x * 2 - 2) + 2.5949095) +
                2) /
               2;
    }
}

static double easeInElastic(double x) {
    if (x == 0 || x == 1) {
        return x;
    }
    return -std::pow(2, (10 * x - 10)) *
           std::sin((x * 10 - 10.75) * 2.0943951023931953);
}

static double easeOutElastic(double x) {
    if (x == 0 || x == 1) {
        return x;
    }
    return std::pow(2, (-10 * x)) *
               std::sin((x * 10 - 0.75) * 2.0943951023931953) +
           1;
}

static double easeInOutElastic(double x) {
    if (x == 0 || x == 1) {
        return x;
    }
    if (x < 0.5) {
        return -(std::pow(2, (20 * x - 10)) *
                 std::sin((20 * x - 11.125) * 1.3962634015954636)) /
               2;
    } else {
        return (std::pow(2, (-20 * x + 10)) *
                std::sin((20 * x - 11.125) * 1.3962634015954636)) /
                   2 +
               1;
    }
}

static double easeOutBounce(double x) {
    double n1 = 7.5625;
    double d1 = 2.75;

    if (x < 1 / d1) {
        return n1 * x * x;
    } else if (x < 2 / d1) {
        x = x - 1.5 / d1;
        return n1 * x * x + 0.75;
    } else if (x < 2.5 / d1) {
        x = x - 2.25 / d1;
        return n1 * x * x + 0.9375;
    } else {
        x = x - 2.625 / d1;
        return n1 * x * x + 0.984375;
    }
}

static double easeInBounce(double x) { return 1 - easeOutBounce(1 - x); }

static double easeInOutBounce(double x) {
    if (x < 0.5) {
        return (1 - easeOutBounce(1 - 2 * x)) / 2;
    } else {
        return (1 + easeOutBounce(2 * x - 1)) / 2;
    }
}

static std::array<uint8_t, 4> over(double r1, double g1, double b1, double a1,
                                   double r2, double g2, double b2, double a2) {
    a1 = a1 / 255.;
    a2 = a2 / 255.;
    double t = a1 * (1 - a2);
    double a = a2 + t;
    if (a == 0) {
        return {0, 0, 0, 0};
    }
    double r = ((r2 / 255.) * a2 + (r1 / 255.) * t) / a;
    double g = ((g2 / 255.) * a2 + (g1 / 255.) * t) / a;
    double b = ((b2 / 255.) * a2 + (b1 / 255.) * t) / a;
    return {(uint8_t)(r * 255.), (uint8_t)(g * 255.), (uint8_t)(b * 255.),
            (uint8_t)(a * 255.)};
}

static constexpr std::array<uint8_t, 4> extractRGBA(uint32_t num) {
    return {(uint8_t)(num >> 16), (uint8_t)(num >> 8), (uint8_t)num,
            (uint8_t)(num >> 24)};
}

static constexpr uint32_t over(uint32_t num1, uint32_t num2) {
    if (num2 >> 24 == 255 || num1 >> 24 == 0)
        return num2;
    auto [r2, g2, b2, a2] = extractRGBA(num2);
    if (a2 == 0)
        return num1;
    auto [r1, g1, b1, a1] = extractRGBA(num1);
    auto [r3, g3, b3, a3] = over(r1, g1, b1, a1, r2, g2, b2, a2);
    return makePixel(r3, g3, b3, a3);
}

static double srgbToLinear(double x) {
    if (x < 0.04045) {
        return x * 0.0773993808;
    }

    return std::pow(x * 0.9478672986 + 0.0521327014, 2.4);
}

static double linearToSrgb(double x) {
    if (x < 0.0031308) {
        return x * 12.92;
    }

    return 1.055 * std::pow(x, 0.41666) - 0.055;
}

static std::array<double, 3> rgbToOklab(double r, double g, double b) {
    r = srgbToLinear(r / 255.);
    g = srgbToLinear(g / 255.);
    b = srgbToLinear(b / 255.);

    double l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b;
    double m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b;
    double s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b;
    l = std::cbrt(l);
    m = std::cbrt(m);
    s = std::cbrt(s);
    return {l * 0.2104542553 + m * 0.7936177850 + s * -0.0040720468,
            l * 1.9779984951 + m * -2.4285922050 + s * 0.4505937099,
            l * 0.0259040371 + m * 0.7827717662 + s * -0.8086757660};
}

static std::array<uint8_t, 3> oklabToRgb(double L, double a, double b) {
    double l = L + a * 0.3963377774 + b * 0.2158037573;
    double m = L + a * -0.1055613458 + b * -0.0638541728;
    double s = L + a * -0.0894841775 + b * -1.2914855480;
    l = std::pow(l, 3);
    m = std::pow(m, 3);
    s = std::pow(s, 3);
    double r = l * 4.0767416621 + m * -3.3077115913 + s * 0.2309699292;
    double g = l * -1.2684380046 + m * 2.6097574011 + s * -0.3413193965;
    b = l * -0.0041960863 + m * -0.7034186147 + s * 1.7076147010;
    r = 255 * saturate(linearToSrgb(r));
    g = 255 * saturate(linearToSrgb(g));
    b = 255 * saturate(linearToSrgb(b));
    return {(uint8_t)r, (uint8_t)g, (uint8_t)b};
}

static std::array<uint8_t, 4> mixColor(uint8_t r1, uint8_t g1, uint8_t b1,
                                       uint8_t a1, uint8_t r2, uint8_t g2,
                                       uint8_t b2, uint8_t a2, double x) {
    auto [l1, oa1, ob1] = rgbToOklab(r1, g1, b1);
    auto [l2, oa2, ob2] = rgbToOklab(r2, g2, b2);
    auto [r, g, b] =
        oklabToRgb(mix(x, l1, l2), mix(x, oa1, oa2), mix(x, ob1, ob2));
    return {r, g, b, (uint8_t)mix(x, a1, a2)};
}
