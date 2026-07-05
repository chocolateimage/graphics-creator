#include "animatable.hpp"
#include "property.hpp"

void Animatable::addProperty(PropertyBase *property) {
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
