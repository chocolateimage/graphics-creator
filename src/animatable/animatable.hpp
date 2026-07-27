#pragma once
#include "frame_info.hpp"
#include <QObject>

class PropertyBase;
class PropertyRenderBase;
class AnimatableRender;

// For timeline
class ICollapsible {
  public:
    virtual bool isCollapsed() = 0;
    virtual void setCollapsed(bool newValue) = 0;
    virtual QString displayName() = 0;
};

class Animatable : public QObject {
    Q_OBJECT
  public:
    void addProperty(PropertyBase *property);
    std::vector<PropertyBase *> properties;

    virtual AnimatableRender *createClass() = 0;
    virtual AnimatableRender *toRender(const FrameInfo &frameInfo);
    virtual QJsonObject serialize();
    virtual void deserialize(const QJsonObject &obj);

    virtual void _propertyUpdated(PropertyBase *property);
    virtual void _propertyIsAnimatingUpdated(PropertyBase *property);

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
