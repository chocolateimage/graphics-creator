#include "element.hpp"
#include <QDebug>

Rect ElementRender::getRenderBox() {
    return {x.get(), y.get(), w.get(), h.get()};
}

Element::~Element() {
    for (auto effect : effects) {
        delete effect;
    }
}
