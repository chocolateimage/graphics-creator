#include "plugin.hpp"
#include "animatable/effect/effect_list.hpp"
#include "animatable/effect/plugin_effect.hpp"
#include <QMessageBox>

void pluginError(const QString &msg) {
    qCritical() << "Plugin error" << qPrintable(msg);
    QMessageBox::critical(nullptr, "Plugin error", msg);
    abort();
}

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

PluginFunctions *createFunctions() {
    PluginFunctions *funcs = new PluginFunctions();
    funcs->log = nullptr; // TODO: log function
    funcs->messageBox = messageBox_def;
    funcs->createEffect = createEffect_def;
    funcs->setEffectRenderFunc = setEffectRenderFunc_def;
    funcs->addEffectProperty = addEffectProperty_def;
    funcs->getEffectProperty = getEffectProperty_def;
    funcs->getPropertyInt = getPropertyInt_def;
    // funcs.setEffectPropertyValue = setEffectPropertyValue_def;
    return funcs;
}

PluginInterface *pluginInterface = nullptr;
