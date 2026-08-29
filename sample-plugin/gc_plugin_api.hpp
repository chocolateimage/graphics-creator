/*
 * ---------------------------
 * Graphics Creator Plugin API
 * ---------------------------
 */

#pragma once
#include <array>
#include <cstdint>

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT
#endif

typedef void Effect;
typedef void Property;
typedef void ComputedProperty;
typedef void Brush;
typedef void ElementSelection;

struct PluginInitData;
struct PluginInterface;

struct Rect {
    int x, y, w, h;
};

struct Color {
    int r{255}, g{255}, b{255}, a{255};
};

struct Vector2DInt {
    int x, y;
};

struct ElementSelectionSnippet {
    Rect rect;
    const uint32_t *__restrict__ values;
};

enum MessageBoxIcon {
    MSGBOX_NONE = 0,
    MSGBOX_INFO,
    MSGBOX_WARN,
    MSGBOX_ERROR,
};

enum PropertyType {
    PROPERTY_TYPE_INT = 0,
    PROPERTY_TYPE_DOUBLE,
    PROPERTY_TYPE_COLOR,
    PROPERTY_TYPE_VECTOR2DINT,
    PROPERTY_TYPE_BOOL,
    PROPERTY_TYPE_BRUSH,
    PROPERTY_TYPE_ELEMENT_SELECTION,
};

enum SetPropertyType {
    SET_DEFAULT = 0,
    SET_MIN,
    SET_MAX,
};

#if NDEBUG
#else
// NOLINTBEGIN
extern "C" {
PLUGIN_EXPORT extern void exportedDebugFunction() {}
}
// NOLINTEND
#endif

struct PluginEffectRenderContext {
    void *privateData;
    Rect renderBox;
    int currentFrame;
    double currentSeconds;
};

typedef void
PluginEffectGetRenderBoxFunc_t(PluginInterface *pluginInterface,
                               PluginEffectRenderContext *renderContext,
                               const Rect &lastBox);

typedef bool PluginEffectRenderFunc_t(PluginInterface *pluginInterface,
                                      PluginEffectRenderContext *renderContext,
                                      const uint32_t *source,
                                      const Rect &sourceRect, uint32_t *target);

struct PluginFunctions {
    void (*log)(const char *msg);
    int (*messageBox)(const char *msg, const char *title, MessageBoxIcon icon,
                      int reserved);

    Effect *(*createEffect)(PluginInitData *initData, const char *category,
                            const char *name, const char *displayName);

    void (*setEffectGetRenderBoxFunc)(Effect *effect,
                                      PluginEffectGetRenderBoxFunc_t func);
    void (*setEffectRenderFunc)(Effect *effect, PluginEffectRenderFunc_t func);

    Property *(*addEffectProperty)(Effect *effect, PropertyType type,
                                   const char *name);
    ComputedProperty *(*getEffectProperty)(
        PluginEffectRenderContext *renderContext, const char *name);

    int (*getPropertyInt)(ComputedProperty *property);
    double (*getPropertyDouble)(ComputedProperty *property);
    Color (*getPropertyColor)(ComputedProperty *property);
    Vector2DInt (*getPropertyVector2DInt)(ComputedProperty *property);
    bool (*getPropertyBool)(ComputedProperty *property);
    Brush *(*getPropertyBrush)(ComputedProperty *property);

    Color (*getBrushPixel)(Brush *brush, int x, int y, int w, int h);

    void (*setPropertyInt)(Property *property, SetPropertyType type, int value);
    void (*setPropertyDouble)(Property *property, SetPropertyType type,
                              double value);
    void (*setPropertyColor)(Property *property, SetPropertyType type,
                             Color color);
    void (*setPropertyVector2DInt)(Property *property, SetPropertyType type,
                                   Vector2DInt value);
    void (*setPropertyBool)(Property *property, SetPropertyType type,
                            bool value);

    void (*addPropertyMenuItem)(Property *property, const char *label);

    ElementSelection *(*getPropertyElementSelection)(
        ComputedProperty *property);
    ElementSelectionSnippet (*getSnippet)(
        PluginEffectRenderContext *renderContext,
        ElementSelection *elementSelection);
};

struct PluginInterface {
    PluginFunctions *functions;
};

struct PluginInitData {
    void *privateData;
    const char *id;
    const char *name;
    const char *version;
};

inline constexpr uint32_t makePixel(uint8_t red, uint8_t green, uint8_t blue,
                                    uint8_t alpha) {
    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

inline constexpr int pixelIndex(int x, int y, int stride) {
    return y * stride + x;
}

static constexpr std::array<uint8_t, 4> extractRGBA(uint32_t num) {
    return {(uint8_t)(num >> 16), (uint8_t)(num >> 8), (uint8_t)num,
            (uint8_t)(num >> 24)};
}
