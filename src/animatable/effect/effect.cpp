#include "effect.hpp"
#include <QJsonObject>

QJsonObject Effect::serialize() {
    QJsonObject obj = Animatable::serialize();
    obj["effectType"] = effectName();
    obj["collapsed"] = collapsed;
    return obj;
}

void Effect::setCollapsed(bool newValue) {
    if (collapsed == newValue)
        return;

    collapsed = newValue;
    emit collapsedChanged(collapsed);
}

void Effect::deserialize(const QJsonObject &obj) {
    Animatable::deserialize(obj);
    collapsed = obj["collapsed"].toBool();
}

Rect EffectRender::getRenderBox(const Rect &lastBox) { return lastBox; }
