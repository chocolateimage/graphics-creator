#include "brush.hpp"
#include "math.hpp"
#include <cmath>

Color getBrushPixel(const Brush &brush, int x, int y, int w, int h) {
    switch (brush.brushType) {
    case Brush::SingleColor:
        return brush.color1;
    case Brush::LinearGradient: {
        double uvX = (double)x / w - 0.5;
        double uvY = (double)y / h - 0.5;
        float rad = -brush.angle * M_PI / 180.f;
        float value =
            saturate(cos(rad + std::atan2(uvY, uvX)) * length(uvX, uvY) + 0.5);
        auto [r, g, b, a] =
            mixColor(brush.color1.r, brush.color1.g, brush.color1.b,
                     brush.color1.a, brush.color2.r, brush.color2.g,
                     brush.color2.b, brush.color2.a, value);
        return {r, g, b, a};
    }
    default: // TODO: add brush types
        return {0, 0, 0, 0};
    }
}
