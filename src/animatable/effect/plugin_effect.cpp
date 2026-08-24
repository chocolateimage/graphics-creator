#include "plugin_effect.hpp"

PluginEffectRender::PluginEffectRender(PluginEffectInfo *info) : info(info) {
    for (const auto &definition : info->properties) {
        PropertyRenderBase *property = nullptr;
        switch (definition->type) {
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

        pluginProperties[definition->name] = property;
    }
}

PluginEffectRender::~PluginEffectRender() { qDeleteAll(pluginProperties); }

template <typename T>
PropertyBase *
PluginEffect::createProperty(PluginPropertyDefinition *definition) {
    auto property = new Property<T>(this, definition->name.toStdString(),
                                    std::get<T>(definition->defaultValue));
    if (definition->hasMin) {
        property->hasMin = true;
        property->min = std::get<T>(definition->min);
    }
    if (definition->hasMax) {
        property->hasMax = true;
        property->max = std::get<T>(definition->max);
    }
    return property;
}

PluginEffect::PluginEffect(PluginEffectInfo *info) : info(info) {
    for (const auto &definition : info->properties) {
        PropertyBase *property = nullptr;
        switch (definition->type) {
        case PROPERTY_TYPE_INT:
            property = createProperty<int>(definition);
            break;
        case PROPERTY_TYPE_DOUBLE:
            property = createProperty<double>(definition);
            break;
        case PROPERTY_TYPE_COLOR:
            property = createProperty<Color>(definition);
            break;
        case PROPERTY_TYPE_VECTOR2DINT:
            property = createProperty<Vector2DInt>(definition);
            break;
        case PROPERTY_TYPE_BOOL:
            property = createProperty<bool>(definition);
            break;
        case PROPERTY_TYPE_BRUSH:
            property = createProperty<Brush>(definition);
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
