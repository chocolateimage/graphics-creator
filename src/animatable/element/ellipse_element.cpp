#include "ellipse_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"

EllipseElement::EllipseElement() : Element() {}

AnimatableRender *EllipseElement::createClass() {
    return new EllipseElementRender();
}

bool EllipseElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    int w = this->w;
    int h = this->h;

    float stepDistance = std::min(1. / w, 1. / h);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Color c = getBrushPixel(fill.get(), x, y, w, h);
            float d = distance((double)x / w, (double)y / h, .5, .5);

            c.a *= linearstep(.5, .5 - stepDistance, d);

            target[pixelIndex(x, y, rect.w)] = makePixel(c);
        }
    }

    return true;
}
