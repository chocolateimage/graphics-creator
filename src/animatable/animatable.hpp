#pragma once
#include <QObject>

class PropertyBase;
class PropertyRenderBase;
class AnimatableRender;

class Animatable : public QObject {
    Q_OBJECT
  public:
    void addProperty(PropertyBase *property);
    std::vector<PropertyBase *> properties;

    virtual AnimatableRender *createClass() = 0;
    AnimatableRender *toRender();
};

class AnimatableRender : public QObject {
    Q_OBJECT
  public:
    void addProperty(PropertyRenderBase *property);
    std::vector<PropertyRenderBase *> properties;
};
