#include "channel_suite.hpp"
#include "ae/smart.hpp"
#include "animatable/effect/ae_plugin_effect.hpp"

int32_t getLayerChannelCount(AEPluginEffectRender *effect, int32_t paramIndex,
                             uint32_t *channels) {
    qInfo() << "getLayerChannelCount";
    *channels = 0;
    return 0;
}

int32_t getLayerChannelIndexedRefAndDesc(AEPluginEffectRender *effect,
                                         int32_t paramIndex,
                                         int32_t channelIndex) {
    qInfo() << "getLayerChannelIndexedRefAndDesc";
    return 0;
}

int32_t getLayerChannelTypedRefAndDesc(AEPluginEffectRender *effect,
                                       int32_t paramIndex, int32_t channelType,
                                       bool *found, void *, void *) {
    qInfo() << "getLayerChannelTypedRefAndDesc";
    *found = false;
    return 0;
}

int32_t checkoutLayerChannel(AEPluginEffectRender *effect, void *,
                             int32_t whatTime, int32_t duration,
                             uint32_t timeScale, int32_t dataType,
                             void *channel) {
    qInfo() << "checkoutLayerChannel";
    return 0;
}

int32_t checkinLayerChannel(AEPluginEffectRender *effect, void *, void *) {
    qInfo() << "checkinLayerChannel";
    return 0;
}

PFChannelSuite1::PFChannelSuite1() {
    getLayerChannelCount = (void *)::getLayerChannelCount;
    getLayerChannelIndexedRefAndDesc =
        (void *)::getLayerChannelIndexedRefAndDesc;
    getLayerChannelTypedRefAndDesc = (void *)::getLayerChannelTypedRefAndDesc;
    checkoutLayerChannel = (void *)::checkoutLayerChannel;
    checkinLayerChannel = (void *)::checkinLayerChannel;
}
