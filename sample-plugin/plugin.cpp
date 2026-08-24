#include "plugin.hpp"
#include "gc_plugin_api.hpp"

void getRenderBox(PluginInterface *intf,
                  PluginEffectRenderContext *renderContext,
                  const Rect &lastBox) {
    int strength = intf->functions->getPropertyInt(
        intf->functions->getEffectProperty(renderContext, "strength"));
    renderContext->renderBox = {lastBox.x - strength, lastBox.y - 30, lastBox.w,
                                lastBox.h};
}

bool render(PluginInterface *intf, PluginEffectRenderContext *renderContext,
            const uint32_t *source, const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderContext->renderBox;
    int strength = intf->functions->getPropertyInt(
        intf->functions->getEffectProperty(renderContext, "strength"));
    auto foregroundBrush = intf->functions->getPropertyBrush(
        intf->functions->getEffectProperty(renderContext, "foreground"));
    if (strength < 1)
        strength = 1;

    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);
            Color c = intf->functions->getBrushPixel(
                foregroundBrush, x, y, sourceRect.w, sourceRect.h);
            target[pixelIndex(x, y, renderContext->renderBox.w)] =
                makePixel(c.r, c.g, c.b, c.a);
        }
    }
    return true;
}

int gcPluginInit(PluginInterface *intf, PluginInitData *data) {
    data->id = "samplePlugin";
    data->name = "Sample Plugin";
    data->version = "1.0.0";

    Effect *effect = intf->functions->createEffect(
        data, "Sample Plugin", "sampleEffect", "Sample Effect");
    intf->functions->setEffectGetRenderBoxFunc(effect, getRenderBox);
    intf->functions->setEffectRenderFunc(effect, render);
    intf->functions->addEffectProperty(effect, PROPERTY_TYPE_INT, "strength");
    intf->functions->addEffectProperty(effect, PROPERTY_TYPE_DOUBLE,
                                       "multiplier");
    intf->functions->addEffectProperty(effect, PROPERTY_TYPE_BOOL, "isSmooth");
    intf->functions->addEffectProperty(effect, PROPERTY_TYPE_COLOR,
                                       "background");
    intf->functions->addEffectProperty(effect, PROPERTY_TYPE_BRUSH,
                                       "foreground");
    intf->functions->addEffectProperty(effect, PROPERTY_TYPE_VECTOR2DINT,
                                       "location");
    return 0;
}
