#include "element.hpp"

Rect Element::getRenderBox() { return {x, y, w, h}; }

Element::~Element() {
    for (auto effect : effects) {
        delete effect;
    }
}

RectangleElement::RectangleElement()
    : Element() {

      };

bool RectangleElement::render(uint32_t *target) {
    auto rect = getRenderBox();
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            target[pixelIndex(x, y, rect.w)] = makePixel(54, 133, 201, 255);
        }
    }

    return true;
}
