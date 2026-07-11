#pragma once
#include "frame_info.hpp"
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
    AnimatableRender *toRender(const FrameInfo &frameInfo);

    void _propertyUpdated(PropertyBase *property);
    void _propertyIsAnimatingUpdated(PropertyBase *property);

  signals:
    void propertyUpdated(PropertyBase *property);
    void propertyIsAnimatingUpdated(PropertyBase *property);
};

class AnimatableRender : public QObject {
    Q_OBJECT
  public:
    void addProperty(PropertyRenderBase *property);
    std::vector<PropertyRenderBase *> properties;
};
