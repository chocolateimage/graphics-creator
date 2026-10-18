#include "aegp.hpp"
#include "ae/suite_loader.hpp"
#include "plugin.hpp"
#include <QDebug>
#include <QMessageBox>

typedef int64_t (*aegpEntrypointFunc_t)(SuiteLoader *, int32_t, int32_t,
                                        int32_t, void **);

QMap<int, AEGP *> AEGP::loadedAegps = {};

AEGP *AEGP::getPlugin(int32_t pluginId) { return loadedAegps[pluginId]; }

AEGP::AEGP(Plugin *plugin, PluginPipl *pipl) : plugin(plugin) {
    if (pipl->properties.contains("name")) {
        plugin->name = pipl->getString("name");
    }
    if (!pipl->properties.contains("8664")) {
        qCritical() << "Could not find entrypoint key";
        QMessageBox::critical(nullptr, "Plugin error",
                              "Could not find entrypoint key in PiPL.");
        return;
    }

    QString entrypoint = pipl->properties["8664"].data();
    aegpEntrypointFunc_t entrypointFunc =
        (aegpEntrypointFunc_t)getLibraryFunction(plugin->library,
                                                 qUtf8Printable(entrypoint));
    if (!entrypointFunc) {
        qCritical() << "Could not find entrypoint" << entrypoint;
        QMessageBox::critical(nullptr, "Plugin error",
                              "Could not find AEGP entrypoint " + entrypoint +
                                  " as defined in PiPL.");
        return;
    }

    suiteLoader = new SuiteLoader();
    pluginId = loadedAegps.size();
    loadedAegps[pluginId] = this;

    entrypointFunc(suiteLoader, 999, 999, pluginId, &globalRefcon);
}
