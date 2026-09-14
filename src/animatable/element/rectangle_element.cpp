#include "rectangle_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"

RectangleElement::RectangleElement() : Element() {
    strokeWidth.setMin(0);
    roundness.setMin(0);
};

AnimatableRender *RectangleElement::createClass() {
    return new RectangleElementRender();
}

Rect RectangleElementRender::getRenderBox() {
    return {x - strokeWidth, y - strokeWidth, w + strokeWidth * 2,
            h + strokeWidth * 2};
}

float roundedRect(float pX, float pY, float bX, float bY, float r) {
    float dX = std::abs(pX) - bX + r;
    float dY = std::abs(pY) - bY + r;
    return length(std::max(dX, 0.f), std::max(dY, 0.f)) +
           std::min(std::max(dX, dY), 0.f) - r;
}

bool RectangleElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    int w = this->w;
    int h = this->h;
    int strokeWidth = this->strokeWidth;
    int roundness = std::min(this->roundness.get(), std::min(h / 2, w / 2));
    auto fill = this->fill;
    auto stroke = this->stroke;
    bool hasStroke = strokeWidth > 0;

    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            int sx = x - strokeWidth;
            int sy = y - strokeWidth;

            float sdf = (strokeWidth == 0 && roundness == 0)
                            ? -.5
                            : roundedRect(sx - w * 0.5f, sy - h * 0.5f,
                                          w * 0.5f, h * 0.5f, roundness);
            Color fc = getBrushPixel(fill, sx, sy, w, h);
            float fillValue = 1 - linearstep(-.5, .5, sdf);
            float totalValue = hasStroke ? 1 - linearstep(-.5 + strokeWidth,
                                                          .5 + strokeWidth, sdf)
                                         : 0;
            float strokeValue = hasStroke ? totalValue - fillValue : 0;

            if (strokeValue > 0) {
                Color sc = getBrushPixel(stroke, x, y, rect.w, rect.h);
                target[pixelIndex(x, y, rect.w)] = makePixel(
                    mix(strokeValue, fc.r, sc.r), mix(strokeValue, fc.g, sc.g),
                    mix(strokeValue, fc.b, sc.b),
                    mix(strokeValue, fc.a, sc.a) * totalValue);
            } else {
                fc.a *= fillValue;
                target[pixelIndex(x, y, rect.w)] = makePixel(fc);
            }
        }
    }

    return true;
}
