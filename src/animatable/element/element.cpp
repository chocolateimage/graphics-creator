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
    connect(effect, &Effect::propertyUpdated, this,
            [this, effect](PropertyBase *property) {
                emit effectPropertyUpdated(effect, property);
            });
    effects.insert(effects.begin() + index, effect);
    emit effectAdded(effect, index);
    emit effectListUpdated();
}

void Element::removeEffect(Effect *effect) {
    effects.erase(std::find(effects.begin(), effects.end(), effect));
    emit effectRemoved(effect);
    emit effectListUpdated();
    delete effect;
}

Element::~Element() {
    for (auto effect : effects) {
        delete effect;
    }
}
