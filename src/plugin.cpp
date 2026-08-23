#include "plugin.hpp"
#include "animatable/effect/effect_list.hpp"
#include "animatable/effect/plugin_effect.hpp"
#include "brush.hpp"
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

void pluginError(const QString &msg) {
    qCritical() << "Plugin error" << qPrintable(msg);
    QMessageBox::critical(nullptr, "Plugin error", msg);
    abort();
}

void log_def(const char *msg) { qInfo() << "[PLUGIN LOG]" << msg; }

void messageBox_def(const char *msg, const char *title, MessageBoxIcon icon) {
    QMessageBox msgBox((QMessageBox::Icon)icon, title, msg,
                       QMessageBox::NoButton, nullptr);
    msgBox.exec();
}

PluginEffectInfo *createEffect_def(PluginInitData *initData,
                                   const char *category, const char *_name,
                                   const char *displayName) {
    if (initData->id == nullptr) {
        pluginError("createEffect: ID not set");
    }

    QString name = QString(initData->id) + ":" + QString(_name);

    PluginEffectInfo *info = new PluginEffectInfo();
    info->name = name;

    EffectInfo effectInfo;
    effectInfo.category = category;
    effectInfo.name = name;
    effectInfo.displayName = displayName;
    effectInfo.create = [info]() {
        PluginEffect *pluginEffect = new PluginEffect(info);
        return pluginEffect;
    };
    effectList.push_back(effectInfo);

    return info;
}

void setEffectRenderFunc_def(PluginEffectInfo *effect,
                             PluginEffectRenderFunc_t func) {
    if (!effect) {
        pluginError("effect is null");
    }
    effect->renderFunc = func;
}

void setEffectGetRenderBoxFunc_def(PluginEffectInfo *effect,
                                   PluginEffectGetRenderBoxFunc_t func) {
    if (!effect) {
        pluginError("effect is null");
    }
    effect->getRenderBoxFunc = func;
}

void addEffectProperty_def(PluginEffectInfo *effect, PropertyType type,
                           const char *name) {
    if (!effect) {
        pluginError("effect is null");
    }
    PluginPropertyDefinition definition;
    definition.type = type;
    definition.name = name;
    effect->properties.append(definition);
}

PropertyRenderBase *
getEffectProperty_def(PluginEffectRenderContext *renderContext,
                      const char *name) {
    auto it = renderContext->privateData->pluginProperties.find(QString(name));
    if (it == renderContext->privateData->pluginProperties.end()) {
        return nullptr;
    }
    return it.value();
}

int getPropertyInt_def(PropertyRenderBase *property) {
    return ((PropertyRender<int> *)property)->get();
}

double getPropertyDouble_def(PropertyRenderBase *property) {
    return ((PropertyRender<double> *)property)->get();
}

Color getPropertyColor_def(PropertyRenderBase *property) {
    return ((PropertyRender<Color> *)property)->get();
}

Vector2DInt getPropertyVector2DInt_def(PropertyRenderBase *property) {
    return ((PropertyRender<Vector2DInt> *)property)->get();
}

bool getPropertyBool_def(PropertyRenderBase *property) {
    return ((PropertyRender<bool> *)property)->get();
}

void *getPropertyBrush_def(PropertyRenderBase *property) {
    return &((PropertyRender<Brush> *)property)->value;
}

PluginFunctions *createFunctions() {
    PluginFunctions *funcs = new PluginFunctions();
    funcs->log = log_def;
    funcs->messageBox = messageBox_def;
    funcs->createEffect = createEffect_def;
    funcs->setEffectRenderFunc = setEffectRenderFunc_def;
    funcs->setEffectGetRenderBoxFunc = setEffectGetRenderBoxFunc_def;
    funcs->addEffectProperty = addEffectProperty_def;
    funcs->getEffectProperty = getEffectProperty_def;
    funcs->getPropertyInt = getPropertyInt_def;
    funcs->getPropertyDouble = getPropertyDouble_def;
    funcs->getPropertyColor = getPropertyColor_def;
    funcs->getPropertyVector2DInt = getPropertyVector2DInt_def;
    funcs->getPropertyBool = getPropertyBool_def;
    funcs->getPropertyBrush = getPropertyBrush_def;
    funcs->getBrushPixel = getBrushPixel;
    // funcs.setEffectPropertyValue = setEffectPropertyValue_def;
    return funcs;
}

Library_t loadLibrary(const QString &path) {
#ifdef Q_OS_WIN
    return LoadLibrary(qUtf16Printable(path));
#else
    return dlopen(qUtf8Printable(path), RTLD_NOW | RTLD_LOCAL);
#endif
}

void *getLibraryFunction(Library_t library, const char *functionName) {
#ifdef Q_OS_WIN
    return (void *)GetProcAddress(library, functionName);
#else
    return dlsym(library, functionName);
#endif
}

PluginManager::PluginManager() {
    QDir appData(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    defaultPluginPath = appData.filePath("plugins");
}
PluginManager::~PluginManager() { qDeleteAll(loadedPlugins); }

void PluginManager::loadDefaultPlugins() {
    QDir dir(defaultPluginPath);
    if (!dir.exists()) {
        dir.mkpath(".");
        return;
    }

    for (const auto &file : dir.entryInfoList(QDir::Files)) {
        loadPlugin(file.filePath());
    }
}

bool PluginManager::loadPlugin(const QString &path) {
    Library_t library = loadLibrary(path);
    if (!library) {
        qWarning() << "Error loading plugin" << path;
        return false;
    }

    gcPluginInit_t pluginInit =
        (gcPluginInit_t)getLibraryFunction(library, "gcPluginInit");
    if (!pluginInit) {
        qWarning() << "Could not find gcPluginInit in plugin" << path;
        return false;
    }

    pluginInterface = new PluginInterface();
    pluginInterface->functions = createFunctions();

    PluginInitDataPrivate priv{};
    PluginInitData data{};
    data.privateData = &priv;
    data.id = nullptr;

    int ret = pluginInit(pluginInterface, &data);
    if (ret != 0) {
        qWarning() << "Error in plugin init for" << path << "with code" << ret;
        return false;
    }

    Plugin *plugin = new Plugin();
    plugin->library = library;
    plugin->id = data.id;
    plugin->name = data.name;
    plugin->version = data.version;
    plugin->path = path;
    loadedPlugins.append(plugin);

    return true;
}

PluginManager *pluginManager = nullptr;
PluginInterface *pluginInterface = nullptr;
