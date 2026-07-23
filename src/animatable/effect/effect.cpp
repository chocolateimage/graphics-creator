#include "effect.hpp"
#include <QJsonObject>

QJsonObject Effect::serialize() {
    QJsonObject obj = Animatable::serialize();
    obj["effectType"] = effectName();
    return obj;
}

Rect EffectRender::getRenderBox(const Rect &lastBox) { return lastBox; }
