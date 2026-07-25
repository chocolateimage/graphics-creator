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

    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            int sx = x - strokeWidth;
            int sy = y - strokeWidth;

            float sdf = (strokeWidth == 0 && roundness == 0)
                            ? -.5
                            : roundedRect(sx - w * 0.5f, sy - h * 0.5f,
                                          w * 0.5f, h * 0.5f, roundness);
            Color fc = getBrushPixel(fill, sx, sy, w, h);
            float fv = 1 - linearstep(-.5, .5, sdf);
            // TODO: there's a gap between the stroke and fill
            float sv = strokeWidth > 0
                           ? ((1 - linearstep(-.5 + strokeWidth,
                                              .5 + strokeWidth, sdf)) -
                              fv)
                           : 0;

            fc.a *= fv;

            if (sv > 0) {
                Color sc = getBrushPixel(stroke, x, y, rect.w, rect.h);
                sc.a *= sv;
                target[pixelIndex(x, y, rect.w)] =
                    makePixel(mix(sv, fc.r, sc.r), mix(sv, fc.g, sc.g),
                              mix(sv, fc.b, sc.b), mix(sv, fc.a, sc.a));
            } else {
                target[pixelIndex(x, y, rect.w)] = makePixel(fc);
            }
        }
    }

    return true;
}
