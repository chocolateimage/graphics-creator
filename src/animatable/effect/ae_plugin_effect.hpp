#pragma once
#include "ae/ae_base.hpp"
#include "ae/ae_effect.hpp"
#include "animatable/property.hpp"
#include "effect.hpp"
#include "plugin.hpp"

class SmartLoadedLayer;

class AEPluginEffectRender : public EffectRender {
  public:
    AEPluginEffectRender(AEEffect *effect);
    ~AEPluginEffectRender();

    QList<PropertyRenderBase *> pluginProperties;
    AEEffect *effect;
    std::vector<AEParameter *> params;
    AEMemoryHandle *sequenceData{nullptr};
    AEMemoryHandle *frameData{nullptr};
    Rect lastBox;
    AELayer *output;
    std::map<int, SmartLoadedLayer *> smartLoadedLayers;
    InData *inData{nullptr};
    AEMemoryHandle *smartData{nullptr};
    std::vector<uint32_t *> copies;

    void prepare();
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
    Rect getRenderBox(const Rect &lastBox) override;
};

class AEPluginEffect : public Effect {
  public:
    AEPluginEffect(AEEffect *effect);
    ~AEPluginEffect();

    template <typename T>
    Property<T> *createProperty(AEEffectParameter *parameter);

    QList<PropertyBase *> pluginProperties;
    AEEffect *effect;
    AEMemoryHandle *sequenceData{nullptr};

    QString getOptionsButtonText() override;
    void openOptions() override;
    QString effectName() override;
    AnimatableRender *createClass() override {
        return new AEPluginEffectRender(effect);
    };
    AnimatableRender *toRender(const FrameInfo &frameInfo) override;
};
