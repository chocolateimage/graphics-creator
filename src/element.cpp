#include "element.hpp"

Rect Element::getBoundingBox() { return {x, y, w, h}; }

Element::~Element() {
    for (auto effect : effects) {
        delete effect;
    }
}

RectangleElement::RectangleElement()
    : Element() {

      };
