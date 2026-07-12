#include "element.hpp"
#include <QDebug>

Rect ElementRender::getRenderBox() {
    return {x.get(), y.get(), w.get(), h.get()};
}

AnimatableRender *Element::toRender(const FrameInfo &frameInfo) {
    ElementRender *render = (ElementRender *)Animatable::toRender(frameInfo);
    for (auto effect : effects) {
        EffectRender *effectRender =
            (EffectRender *)effect->toRender(frameInfo);
        effectRender->element = render;
        render->effects.push_back(effectRender);
    }
    return render;
}

void Element::addEffect(Effect *effect) {
    insertEffect(effect, effects.size());
}

void Element::insertEffect(Effect *effect, int index) {
    effects.insert(effects.begin() + index, effect);
    emit effectAdded(effect, index);
}

Element::~Element() {
    for (auto effect : effects) {
        delete effect;
    }
}
