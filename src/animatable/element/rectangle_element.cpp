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
    return {x.get() - strokeWidth.get(), y.get() - strokeWidth.get(),
            w.get() + strokeWidth.get() * 2, h.get() + strokeWidth.get() * 2};
}

float roundedRect(float pX, float pY, float bX, float bY, float r) {
    float dX = std::abs(pX) - bX + r;
    float dY = std::abs(pY) - bY + r;
    return length(std::max(dX, 0.f), std::max(dY, 0.f)) - r;
}

bool RectangleElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    int w = this->w;
    int h = this->h;
    int strokeWidth = this->strokeWidth;
    int roundness = std::min(this->roundness.get(), std::min(h / 2, w / 2));
    auto fill = this->fill.get();

    for (int y = 0; y < strokeWidth; y++) {
        for (int x = 0; x < w + strokeWidth * 2; x++) {
            target[pixelIndex(x, y, rect.w)] = makePixel(0, 0, 0, 255);
            target[pixelIndex(x, y + h + strokeWidth, rect.w)] =
                makePixel(0, 0, 0, 255);
        }
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < strokeWidth; x++) {
            target[pixelIndex(x, y + strokeWidth, rect.w)] =
                makePixel(0, 0, 0, 255);
            target[pixelIndex(x + w + strokeWidth, y + strokeWidth, rect.w)] =
                makePixel(0, 0, 0, 255);
        }
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Color c = getBrushPixel(fill, x, y, w, h);

            if (roundness > 0) {
                float sdf = roundedRect(x - w * 0.5f, y - h * 0.5f, w * 0.5f,
                                        h * 0.5f, roundness);

                c.a *= 1 - linearstep(-.5, .5, sdf);
            }

            target[pixelIndex(x + strokeWidth, y + strokeWidth, rect.w)] =
                makePixel(c);
        }
    }

    return true;
}
