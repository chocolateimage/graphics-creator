#include "plugin.hpp"
#include "ae/ae_effect.hpp"
#include "ae/aegp.hpp"
#include "ae/entry.hpp"
#include "animatable/effect/effect_list.hpp"
#include "animatable/effect/plugin_effect.hpp"
#include "brush.hpp"
#include "render.hpp"
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

int messageBox_def(const char *msg, const char *title, MessageBoxIcon icon,
                   int reserved) {
    QMessageBox msgBox((QMessageBox::Icon)icon, title, msg,
                       QMessageBox::NoButton, nullptr);
    msgBox.exec();
    return 0;
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

PluginPropertyDefinition *addEffectProperty_def(PluginEffectInfo *effect,
                                                PropertyType type,
                                                const char *name) {
    if (!effect) {
        pluginError("effect is null");
    }
    PluginPropertyDefinition *definition = new PluginPropertyDefinition();
    definition->type = type;
    definition->name = name;
    switch (type) {
    case PROPERTY_TYPE_INT:
        definition->defaultValue = 0;
        break;
    case PROPERTY_TYPE_DOUBLE:
        definition->defaultValue = 0.0;
        break;
    case PROPERTY_TYPE_COLOR:
        definition->defaultValue = Color{};
        break;
    case PROPERTY_TYPE_VECTOR2DINT:
        definition->defaultValue = Vector2DInt{0, 0};
        break;
    case PROPERTY_TYPE_BRUSH:
        definition->defaultValue = Brush{};
        break;
    case PROPERTY_TYPE_BOOL:
        definition->defaultValue = false;
        break;
    case PROPERTY_TYPE_ELEMENT_SELECTION:
        definition->defaultValue = ElementSelection{};
        break;
    }
    effect->properties.append(definition);
    return definition;
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
    return &((PropertyRender<Brush> *)property)->get();
}

void *getPropertyElementSelection_def(PropertyRenderBase *property) {
    return &((PropertyRender<ElementSelection> *)property)->get();
}

void setPropertyInt_def(PluginPropertyDefinition *property,
                        SetPropertyType type, int value) {
    property->setValue(value, type);
}

void setPropertyDouble_def(PluginPropertyDefinition *property,
                           SetPropertyType type, double value) {
    property->setValue(value, type);
}

void setPropertyColor_def(PluginPropertyDefinition *property,
                          SetPropertyType type, Color value) {
    property->setValue(value, type);
}

void setPropertyVector2DInt_def(PluginPropertyDefinition *property,
                                SetPropertyType type, Vector2DInt value) {
    property->setValue(value, type);
}

void setPropertyBool_def(PluginPropertyDefinition *property,
                         SetPropertyType type, bool value) {
    property->setValue(value, type);
}

void addPropertyMenuItem_def(PluginPropertyDefinition *property,
                             const char *label) {
    property->menuItems.append(label);
}

ElementSelectionSnippet getSnippet_def(PluginEffectRenderContext *renderContext,
                                       ElementSelection *elementSelection) {
    return renderContext->privateData->renderThread->getSnippet(
        *elementSelection);
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
    funcs->getPropertyElementSelection = getPropertyElementSelection_def;

    funcs->getBrushPixel = getBrushPixel;
    funcs->getSnippet = getSnippet_def;

    funcs->setPropertyInt = setPropertyInt_def;
    funcs->setPropertyDouble = setPropertyDouble_def;
    funcs->setPropertyColor = setPropertyColor_def;
    funcs->setPropertyVector2DInt = setPropertyVector2DInt_def;
    funcs->setPropertyBool = setPropertyBool_def;

    funcs->addPropertyMenuItem = addPropertyMenuItem_def;

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

#ifdef Q_OS_WIN
int PIPL_Entry(HMODULE hModule, LPCSTR lpType, LPSTR lpName, LONG_PTR lParam) {
    PluginInitData *initData = (PluginInitData *)lParam;
    HRSRC resource = FindResourceA(hModule, lpName, lpType);
    HGLOBAL loadedResource = LoadResource(hModule, resource);
    size_t size = SizeofResource(hModule, resource);
    const char *data = (const char *)LockResource(loadedResource);

    PluginPipl *pluginPipl = new PluginPipl(QByteArray(data, size));
    initData->privateData->pipls.append(pluginPipl);
    return 1;
}
#endif

PluginPipl::PluginPipl(const QByteArray &data) {
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.skipRawData(6);
    uint32_t count;
    stream >> count;
    for (uint32_t i = 0; i < count; i++) {
        stream.skipRawData(4);

        char rawKey[5];
        stream >> rawKey[3];
        stream >> rawKey[2];
        stream >> rawKey[1];
        stream >> rawKey[0];
        rawKey[4] = 0;
        stream.skipRawData(4);
        QString key = QString(rawKey);

        uint32_t length;
        stream >> length;

        QByteArray value(length, 0);
        stream.readRawData(value.data(), length);

        properties[key] = value;
    }
}

QString PluginPipl::getString(const QString &key) {
    QByteArray bytes = properties[key];
    uint8_t length = bytes[0];
    return bytes.sliced(1, length);
}

bool PluginManager::loadPlugin(const QString &path) {
    qInfo() << "Loading plugin" << path;
    Library_t library = loadLibrary(path);
    if (!library) {
        qWarning() << "Error loading plugin" << path;
        QMessageBox::warning(nullptr, "Plugin error",
                             "Error loading plugin " + path);
        return false;
    }

    if (!pluginInterface) {
        pluginInterface = new PluginInterface();
        pluginInterface->functions = createFunctions();
    }

    PluginInitDataPrivate priv{};
    priv.library = library;
    PluginInitData data{};
    data.privateData = &priv;
    data.id = nullptr;
    data.name = nullptr;
    data.version = nullptr;

#ifdef Q_OS_WIN
    EnumResourceNamesA(library, "PIPL", PIPL_Entry, (LONG_PTR)&data);
#endif

    void *aeEntry = getLibraryFunction(library, "PluginDataEntryFunction");
    void *aeEntry2 = getLibraryFunction(library, "PluginDataEntryFunction2");
    void *gcInit = getLibraryFunction(library, "gcPluginInit");
    if (gcInit) {
        gcPluginInit_t pluginInit = (gcPluginInit_t)gcInit;

        int ret = pluginInit(pluginInterface, &data);
        if (ret != 0) {
            qWarning() << "Error in plugin init for" << path << "with code"
                       << ret;
            return false;
        }
    } else if (aeEntry || aeEntry2) {
        if (aeEntry2) {
            aeEntry = aeEntry2;
        }
        qInfo() << "Calling entry";
        aeEntryFunc_t aeEntryFunc = (aeEntryFunc_t)aeEntry;
        int ret = aeEntryFunc(&data, aeCallback);
        qInfo() << "Called entry";
        if (ret != 0) {
            qWarning() << "Error in plugin init for" << path << "with code"
                       << ret;
            return false;
        }
    } else if (data.privateData->pipls.isEmpty()) {
        qWarning() << "Could not find gcPluginInit in plugin" << path;
        return false;
    }

    Plugin *plugin = new Plugin();
    plugin->library = library;
    plugin->id = data.id;
    if (data.name) {
        plugin->name = data.name;
    } else {
        plugin->name = "<unknown>";
    }
    if (data.version) {
        plugin->version = data.version;
    } else {
        plugin->version = "<unknown>";
    }
    plugin->path = path;
    for (auto effect : data.privateData->aeEffects) {
        plugin->aeEffects.push_back(effect);
        effect->plugin = plugin;
        effect->registerEffect();
    }
    for (auto pipl : data.privateData->pipls) {
        /*key "kind"
value "xgEA"
key "name"
value "\tPanelator\x00\x00"
key "catg"
value "\x0EGeneral Plugin\x00"
key "vers"
value "\x00\x00\x03\x00"
key "8664"
value "EntryPointFunc\x00\x00"*/
        if (pipl->properties["kind"] == "xgEA") {
            AEGP *aegp = new AEGP(plugin, pipl);
            plugin->aegps.append(aegp);
        }
        delete pipl;
    }
    plugin->isDebug =
        getLibraryFunction(library, "exportedDebugFunction") != nullptr;
    loadedPlugins.append(plugin);

    if (plugin->isDebug) {
        qWarning() << "Plugin" << plugin->name << "is running in debug mode";
    }

    return true;
}

int PluginManager::aeCallback(void *data, const char *_name, const char *,
                              const char *_category, const char *funcName,
                              int32_t, int32_t ver1, int32_t ver2, int32_t) {
    PluginInitData *initData = (PluginInitData *)data;

    initData->id = _name;

    if (_name[0] == '$' && _name[1] == '$' && _name[2] == '$') {
        initData->name = strchr(_name, '=') + 1;
    } else {
        initData->name = _name;
    }

    initData->version = "AE";

    AEEffect *aeEffect = new AEEffect();
    aeEffect->name = _name;
    aeEffect->displayName = initData->name;
    aeEffect->effectEntryFunc = (effectEntryFunc_t)getLibraryFunction(
        initData->privateData->library, funcName);

    if (_category[0] == '$' && _category[1] == '$' && _category[2] == '$') {
        aeEffect->category = strchr(_category, '=') + 1;
    } else {
        aeEffect->category = _category;
    }

    initData->privateData->aeEffects.push_back(aeEffect);
    return 0;
}

PluginManager *pluginManager = nullptr;
PluginInterface *pluginInterface = nullptr;
