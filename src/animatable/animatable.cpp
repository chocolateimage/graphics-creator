#include "animatable.hpp"
#include "property.hpp"
#include <QJsonObject>

void Animatable::addProperty(PropertyBase *property) {
    properties.push_back(property);
}

void AnimatableRender::addProperty(PropertyRenderBase *property) {
    properties.push_back(property);
}

AnimatableRender *Animatable::toRender(const FrameInfo &frameInfo) {
    AnimatableRender *instance = createClass();
    for (size_t index = 0; index < properties.size(); index++) {
        instance->properties[index]->set(properties[index], frameInfo);
    }
    return instance;
}

void Animatable::_propertyIsAnimatingUpdated(PropertyBase *property) {
    emit propertyIsAnimatingUpdated(property);
}

void Animatable::_propertyUpdated(PropertyBase *property) {
    emit propertyUpdated(property);
}

QJsonObject Animatable::serialize() {
    QJsonObject obj;
    QJsonObject propertiesObject;
    for (auto property : properties) {
        propertiesObject[QString::fromStdString(property->name)] =
            property->serialize();
    }
    obj["properties"] = propertiesObject;
    obj["name"] = objectName();
    return obj;
}

void Animatable::deserialize(const QJsonObject &obj) {
    setObjectName(obj["name"].toString());
    QJsonObject propertiesObj = obj["properties"].toObject();
    for (auto property : properties) {
        QString key = QString::fromStdString(property->name);
        if (!propertiesObj.contains(key)) {
            continue;
        }

        QJsonObject propertyObj = propertiesObj[key].toObject();
    }
}
