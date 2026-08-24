#pragma once
#include "animatable/property.hpp"
#include "effect.hpp"
#include "plugin.hpp"

class PluginEffectRender : public EffectRender {
  public:
    PluginEffectRender(PluginEffectInfo *info);
    ~PluginEffectRender();

    PluginEffectRenderContext getContext();

    QMap<QString, PropertyRenderBase *> pluginProperties;
    PluginEffectInfo *info;

    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;
};

class PluginEffect : public Effect {
  public:
    PluginEffect(PluginEffectInfo *info);
    ~PluginEffect();

    template <typename T>
    Property<T> *createProperty(PluginPropertyDefinition *definition);

    QList<PropertyBase *> pluginProperties;
    PluginEffectInfo *info;

    QString effectName() override;
    AnimatableRender *createClass() override {
        return new PluginEffectRender(info);
    };
};
