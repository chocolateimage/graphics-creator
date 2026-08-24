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
    int type = intf->functions->getPropertyInt(
        intf->functions->getEffectProperty(renderContext, "type"));
    int strength = intf->functions->getPropertyInt(
        intf->functions->getEffectProperty(renderContext, "strength"));
    auto background = intf->functions->getPropertyColor(
        intf->functions->getEffectProperty(renderContext, "background"));
    auto foregroundBrush = intf->functions->getPropertyBrush(
        intf->functions->getEffectProperty(renderContext, "foreground"));

    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);
            Color c = type == 0
                          ? background
                          : intf->functions->getBrushPixel(foregroundBrush, x,
                                                           y, sourceRect.w,
                                                           sourceRect.h);
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

    Effect *effect =
        intf->functions->createEffect(data, "Sample Plugin", "demo", "Demo");
    intf->functions->setEffectGetRenderBoxFunc(effect, getRenderBox);
    intf->functions->setEffectRenderFunc(effect, render);

    Property *type =
        intf->functions->addEffectProperty(effect, PROPERTY_TYPE_INT, "type");
    intf->functions->addPropertyMenuItem(type, "Background");
    intf->functions->addPropertyMenuItem(type, "Foreground");

    Property *strength = intf->functions->addEffectProperty(
        effect, PROPERTY_TYPE_INT, "strength");
    intf->functions->setPropertyInt(strength, SET_MIN, 0);
    intf->functions->setPropertyInt(strength, SET_MAX, 100);
    intf->functions->setPropertyInt(strength, SET_DEFAULT, 20);

    Property *multiplier = intf->functions->addEffectProperty(
        effect, PROPERTY_TYPE_DOUBLE, "multiplier");
    intf->functions->setPropertyDouble(multiplier, SET_DEFAULT, 20);

    Property *isSmooth = intf->functions->addEffectProperty(
        effect, PROPERTY_TYPE_BOOL, "isSmooth");
    intf->functions->setPropertyBool(isSmooth, SET_DEFAULT, true);

    Property *background = intf->functions->addEffectProperty(
        effect, PROPERTY_TYPE_COLOR, "background");
    intf->functions->setPropertyColor(background, SET_DEFAULT,
                                      {23, 111, 227, 255});

    intf->functions->addEffectProperty(effect, PROPERTY_TYPE_BRUSH,
                                       "foreground");

    Property *location = intf->functions->addEffectProperty(
        effect, PROPERTY_TYPE_VECTOR2DINT, "location");
    intf->functions->setPropertyVector2DInt(location, SET_DEFAULT, {300, 400});

    return 0;
}
