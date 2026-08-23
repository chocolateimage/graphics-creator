#pragma once
#include "variant.hpp"
#include <QList>

#ifdef Q_OS_WIN
#include <windows.h>
typedef HMODULE Library_t;
#else
typedef void *Library_t;
#endif

struct PluginInitData;
struct PluginInterface;
class PluginEffectRender;
class PropertyRenderBase;

enum MessageBoxIcon {
    MSGBOX_NONE = 0,
    MSGBOX_INFO,
    MSGBOX_WARN,
    MSGBOX_ERROR,
};

enum PropertyType {
    PROPERTY_TYPE_INT = 0,
    PROPERTY_TYPE_DOUBLE,
    PROPERTY_TYPE_COLOLR,
    PROPERTY_TYPE_VECTOR2DINT,
    PROPERTY_TYPE_BOOL,
    PROPERTY_TYPE_BRUSH,
};

struct PluginEffectRenderContext {
    PluginEffectRender *privateData;
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

class PluginPropertyDefinition {
  public:
    QString name;
    PropertyType type;
};

class PluginEffectInfo {
  public:
    QString name;
    PluginEffectGetRenderBoxFunc_t *getRenderBoxFunc{nullptr};
    PluginEffectRenderFunc_t *renderFunc{nullptr};
    QList<PluginPropertyDefinition> properties;
};

struct PluginFunctions {
    void (*log)(const char *msg);
    void (*messageBox)(const char *msg, const char *title, MessageBoxIcon icon);
    PluginEffectInfo *(*createEffect)(PluginInitData *initData,
                                      const char *category, const char *name,
                                      const char *displayName);
    void (*setEffectRenderFunc)(PluginEffectInfo *effect,
                                PluginEffectRenderFunc_t func);
    void (*addEffectProperty)(PluginEffectInfo *effect, PropertyType type,
                              const char *name);
    PropertyRenderBase *(*getEffectProperty)(
        PluginEffectRenderContext *renderContext, const char *name);
    int (*getPropertyInt)(PropertyRenderBase *property);
    void (*setEffectGetRenderBoxFunc)(PluginEffectInfo *effect,
                                      PluginEffectGetRenderBoxFunc_t func);
};

struct PluginInterface {
    PluginFunctions *functions;
};

struct PluginInitDataPrivate {};

struct PluginInitData {
    PluginInitDataPrivate *privateData;
    const char *id;
    const char *name;
    const char *version;
};

PluginFunctions *createFunctions();

void pluginError(const QString &msg);

Library_t loadLibrary(const QString &path);
void *getLibraryFunction(Library_t library, const char *functionName);

extern PluginInterface *pluginInterface;
