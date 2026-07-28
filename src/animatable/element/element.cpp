#include "element.hpp"
#include "animatable/effect/effect_list.hpp"
#include <QDebug>

Rect ElementRender::getRenderBox() {
    return {x.get(), y.get(), w.get(), h.get()};
}

AnimatableRender *Element::toRender(const FrameInfo &frameInfo) {
    ElementRender *render = (ElementRender *)Animatable::toRender(frameInfo);
    render->id = id;
    render->visible = visible;
    render->startFrame = startFrame;
    render->durationFrames = durationFrames;
    for (auto effect : effects) {
        EffectRender *effectRender =
            (EffectRender *)effect->toRender(frameInfo);
        effectRender->element = render;
        render->effects.push_back(effectRender);
    }
    return render;
}

QRect Element::getBoundingBox(const FrameInfo &frameInfo) {
    return {x.get(frameInfo), y.get(frameInfo), w.get(frameInfo),
            h.get(frameInfo)};
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

void Element::reorderEffect(Effect *effect, int newIndex) {
    int oldIndex = effects.indexOf(effect);
    if (newIndex == oldIndex)
        return;

    int toAddIndex = newIndex;
    if (toAddIndex > oldIndex) {
        toAddIndex--;
    }
    effects.removeOne(effect);
    effects.insert(toAddIndex, effect);

    emit effectListUpdated();
}

void Element::setEditMode(bool newMode) {
    if (newMode == editMode)
        return;

    editMode = newMode;
    emit editModeUpdated(newMode);
}

void Element::setVisible(bool newValue) {
    if (visible == newValue)
        return;

    visible = newValue;
    emit visibilityUpdated(newValue);
}

QJsonObject Element::serialize() {
    QJsonObject obj = Animatable::serialize();
    obj["id"] = id;
    obj["elementType"] = typeName();
    obj["collapsed"] = collapsed;
    obj["visible"] = visible;
    obj["startFrame"] = startFrame;
    obj["durationFrames"] = durationFrames;
    QJsonArray effectsArray;
    for (auto effect : effects) {
        effectsArray.append(effect->serialize());
    }
    obj["effects"] = effectsArray;
    return obj;
}

void Element::deserialize(const QJsonObject &obj) {
    Animatable::deserialize(obj);
    if (obj.contains("id")) {
        id = obj["id"].toString();
    }
    collapsed = obj["collapsed"].toBool();
    visible = obj["visible"].toBool(true);
    if (obj.contains("startFrame") && obj.contains("durationFrames")) {
        startFrame = obj["startFrame"].toInt(0);
        durationFrames = obj["durationFrames"].toInt(1);
    }
    for (auto effectJson : obj["effects"].toArray()) {
        QJsonObject effectObject = effectJson.toObject();
        QString effectType = effectObject["effectType"].toString();
        Effect *effect{nullptr};

        for (const auto &effectInfo : effectList) {
            if (effectInfo.name == effectType) {
                effect = effectInfo.create();
                break;
            }
        }

        if (effect) {
            effect->deserialize(effectObject);
            addEffect(effect);
        }
    }
}

Element::~Element() {
    for (auto effect : effects) {
        delete effect;
    }
}

ElementRender::~ElementRender() {
    for (auto effect : effects) {
        delete effect;
    }
}
