#include "plugin.hpp"
#include "gc_plugin_api.hpp"

bool render(PluginInterface *intf, PluginEffectRenderContext *renderContext,
            const uint32_t *source, const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderContext->renderBox;
    int darkness = intf->functions->getPropertyInt(
        intf->functions->getEffectProperty(renderContext, "darkness"));

    float multiplier = 1 - darkness / 100.;

    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);

            target[pixelIndex(x, y, rect.w)] =
                makePixel(r * multiplier, g * multiplier, b * multiplier, a);
        }
    }
    return true;
}

int gcPluginInit(PluginInterface *intf, PluginInitData *data) {
    data->id = "samplePlugin";
    data->name = "Sample Plugin";
    data->version = "1.0.0";

    Effect *effect =
        intf->functions->createEffect(data, data->name, "darken", "Darken");
    intf->functions->setEffectRenderFunc(effect, render);

    Property *darkness = intf->functions->addEffectProperty(
        effect, PROPERTY_TYPE_INT, "darkness");
    intf->functions->setPropertyInt(darkness, SET_MIN, 0);
    intf->functions->setPropertyInt(darkness, SET_MAX, 100);
    intf->functions->setPropertyInt(darkness, SET_DEFAULT, 50);

    return 0;
}
