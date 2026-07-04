#include "element.hpp"
#include "brush.hpp"
#include "math.hpp"
#include <QDebug>

PropertyBase::PropertyBase(const std::string &name) : name(name) {}

template <typename T>
Property<T>::Property(Animatable *animatable, const std::string &name,
                      T defaultValue)
    : PropertyBase(name), value(defaultValue) {
    animatable->addProperty(this);
}

PropertyRenderBase::PropertyRenderBase() {}

template <typename T>
PropertyRender<T>::PropertyRender(AnimatableRender *animatableRender)
    : PropertyRenderBase() {
    animatableRender->addProperty(this);
}

template <typename T> T Property<T>::get() { return value; }

template <typename T> void PropertyRender<T>::set(PropertyBase *property) {
    auto propertyTyped = dynamic_cast<Property<T> *>(property);
    value = propertyTyped->get();
}

template <typename T> T PropertyRender<T>::get() { return value; }

void Animatable::addProperty(PropertyBase *property) {
    qInfo() << property->name;
    properties.push_back(property);
}

void AnimatableRender::addProperty(PropertyRenderBase *property) {
    properties.push_back(property);
}

AnimatableRender *Animatable::toRender() {
    AnimatableRender *instance = createClass();
    for (size_t index = 0; index < properties.size(); index++) {
        instance->properties[index]->set(properties[index]);
    }
    return instance;
}

Rect ElementRender::getRenderBox() {
    return {x.get(), y.get(), w.get(), h.get()};
}

Element::~Element() {
    for (auto effect : effects) {
        delete effect;
    }
}

RectangleElement::RectangleElement()
    : Element() {

      };

AnimatableRender *RectangleElement::createClass() {
    return new RectangleElementRender();
}

bool RectangleElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            target[pixelIndex(x, y, rect.w)] =
                makePixel(getBrushPixel(fill.get(), x, y, rect.w, rect.h));
        }
    }

    return true;
}
