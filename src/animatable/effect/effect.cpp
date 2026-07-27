#include "effect.hpp"
#include "effect_list.hpp"
#include <QJsonObject>

QJsonObject Effect::serialize() {
    QJsonObject obj = Animatable::serialize();
    obj["effectType"] = effectName();
    obj["collapsed"] = collapsed;
    return obj;
}

bool Effect::isCollapsed() { return collapsed; }

void Effect::setCollapsed(bool newValue) {
    if (collapsed == newValue)
        return;

    collapsed = newValue;
    emit collapsedChanged(collapsed);
}

QString Effect::displayName() {
    QString targetName = this->effectName();
    for (const auto &effect : effectList) {
        if (effect.name == targetName) {
            return effect.displayName;
        }
    }
    return targetName;
}

void Effect::deserialize(const QJsonObject &obj) {
    Animatable::deserialize(obj);
    collapsed = obj["collapsed"].toBool();
}

Rect EffectRender::getRenderBox(const Rect &lastBox) { return lastBox; }
