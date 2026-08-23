#include "plugin_effect.hpp"

PluginEffectRender::PluginEffectRender(PluginEffectInfo *info) : info(info) {
    for (const auto &definition : info->properties) {
        PropertyRenderBase *property = nullptr;
        switch (definition.type) {
        case PROPERTY_TYPE_INT:
            property = new PropertyRender<int>(this);
            break;
        case PROPERTY_TYPE_DOUBLE:
            property = new PropertyRender<double>(this);
            break;
        case PROPERTY_TYPE_COLOR:
            property = new PropertyRender<Color>(this);
            break;
        case PROPERTY_TYPE_VECTOR2DINT:
            property = new PropertyRender<Vector2DInt>(this);
            break;
        case PROPERTY_TYPE_BOOL:
            property = new PropertyRender<bool>(this);
            break;
        case PROPERTY_TYPE_BRUSH:
            property = new PropertyRender<Brush>(this);
            break;
        }

        pluginProperties[definition.name] = property;
    }
}

PluginEffectRender::~PluginEffectRender() { qDeleteAll(pluginProperties); }

PluginEffect::PluginEffect(PluginEffectInfo *info) : info(info) {
    for (const auto &definition : info->properties) {
        PropertyBase *property = nullptr;
        std::string name = definition.name.toStdString();
        switch (definition.type) {
        case PROPERTY_TYPE_INT:
            property = new Property<int>(this, name, 0);
            break;
        case PROPERTY_TYPE_DOUBLE:
            property = new Property<double>(this, name, 0);
            break;
        case PROPERTY_TYPE_COLOR:
            property = new Property<Color>(this, name, {});
            break;
        case PROPERTY_TYPE_VECTOR2DINT:
            property = new Property<Vector2DInt>(this, name, {});
            break;
        case PROPERTY_TYPE_BOOL:
            property = new Property<bool>(this, name, false);
            break;
        case PROPERTY_TYPE_BRUSH:
            property = new Property<Brush>(this, name, {});
            break;
        }

        pluginProperties.append(property);
    }
}

PluginEffect::~PluginEffect() { qDeleteAll(pluginProperties); }

QString PluginEffect::effectName() { return info->name; }

PluginEffectRenderContext PluginEffectRender::getContext() {
    PluginEffectRenderContext ctx;
    ctx.privateData = this;
    ctx.renderBox = renderBox;
    ctx.currentFrame = currentFrame;
    ctx.currentSeconds = currentSeconds;
    return ctx;
}

Rect PluginEffectRender::getRenderBox(const Rect &lastBox) {
    if (info->getRenderBoxFunc) {
        PluginEffectRenderContext ctx = getContext();
        info->getRenderBoxFunc(pluginInterface, &ctx, lastBox);
        return ctx.renderBox;
    }
    return lastBox;
}

bool PluginEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                                uint32_t *target) {
    PluginEffectRenderContext ctx = getContext();

    info->renderFunc(pluginInterface, &ctx, source, sourceRect, target);

    return true;
}
