#include "rectangle_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"

RectangleElement::RectangleElement()
    : Element() {

      };

AnimatableRender *RectangleElement::createClass() {
    return new RectangleElementRender();
}

Rect RectangleElementRender::getRenderBox() {
    return {x.get() - strokeWidth.get(), y.get() - strokeWidth.get(),
            w.get() + strokeWidth.get() * 2, h.get() + strokeWidth.get() * 2};
}

bool RectangleElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    int w = this->w.get();
    int h = this->h.get();
    int strokeWidth = this->strokeWidth.get();

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
            target[pixelIndex(x + strokeWidth, y + strokeWidth, rect.w)] =
                makePixel(getBrushPixel(fill.get(), x, y, w, h));
        }
    }

    return true;
}
