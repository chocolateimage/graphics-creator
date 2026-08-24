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

typedef int (*gcPluginInit_t)(PluginInterface *, PluginInitData *);

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
};

enum SetPropertyType {
    SET_DEFAULT = 0,
    SET_MIN,
    SET_MAX,
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

    void setValue(VariantType val, SetPropertyType setType) {
        switch (setType) {
        case SET_DEFAULT: {
            defaultValue = std::move(val);
            break;
        }
        case SET_MIN: {
            hasMin = true;
            min = std::move(val);
            break;
        }
        case SET_MAX: {
            hasMax = true;
            max = std::move(val);
            break;
        }
        }
    }

    bool hasMin{false};
    VariantType min;
    bool hasMax{false};
    VariantType max;
    VariantType defaultValue;
};

class PluginEffectInfo {
  public:
    QString name;
    PluginEffectGetRenderBoxFunc_t *getRenderBoxFunc{nullptr};
    PluginEffectRenderFunc_t *renderFunc{nullptr};
    QList<PluginPropertyDefinition *> properties;
};

struct PluginFunctions {
    void (*log)(const char *msg);
    int (*messageBox)(const char *msg, const char *title, MessageBoxIcon icon,
                      int reserved);

    PluginEffectInfo *(*createEffect)(PluginInitData *initData,
                                      const char *category, const char *name,
                                      const char *displayName);

    void (*setEffectGetRenderBoxFunc)(PluginEffectInfo *effect,
                                      PluginEffectGetRenderBoxFunc_t func);
    void (*setEffectRenderFunc)(PluginEffectInfo *effect,
                                PluginEffectRenderFunc_t func);

    PluginPropertyDefinition *(*addEffectProperty)(PluginEffectInfo *effect,
                                                   PropertyType type,
                                                   const char *name);
    PropertyRenderBase *(*getEffectProperty)(
        PluginEffectRenderContext *renderContext, const char *name);

    int (*getPropertyInt)(PropertyRenderBase *property);
    double (*getPropertyDouble)(PropertyRenderBase *property);
    Color (*getPropertyColor)(PropertyRenderBase *property);
    Vector2DInt (*getPropertyVector2DInt)(PropertyRenderBase *property);
    bool (*getPropertyBool)(PropertyRenderBase *property);
    void *(*getPropertyBrush)(PropertyRenderBase *property);

    Color (*getBrushPixel)(const Brush &brush, int x, int y, int w, int h);

    void (*setPropertyInt)(PluginPropertyDefinition *property,
                           SetPropertyType type, int value);
    void (*setPropertyDouble)(PluginPropertyDefinition *property,
                              SetPropertyType type, double value);
    void (*setPropertyColor)(PluginPropertyDefinition *property,
                             SetPropertyType type, Color color);
    void (*setPropertyVector2DInt)(PluginPropertyDefinition *property,
                                   SetPropertyType type, Vector2DInt value);
    void (*setPropertyBool)(PluginPropertyDefinition *property,
                            SetPropertyType type, bool value);
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

class Plugin {
  public:
    Library_t library;
    QString id;
    QString name;
    QString version;
    QString path;
    bool isDebug;
};

PluginFunctions *createFunctions();

void pluginError(const QString &msg);

Library_t loadLibrary(const QString &path);
void *getLibraryFunction(Library_t library, const char *functionName);

class PluginManager {
  public:
    PluginManager();
    ~PluginManager();

    QString defaultPluginPath;
    QList<Plugin *> loadedPlugins;

    void loadDefaultPlugins();
    bool loadPlugin(const QString &path);
};

extern PluginManager *pluginManager;
extern PluginInterface *pluginInterface;
