#include "aegp_register_suite.hpp"
#include "ae/ae_base.hpp"
#include "ae/aegp.hpp"
#include "aegp_command_suite.hpp"
#include <QAction>

typedef int32_t (*commandHookFunc_t)(void *globalRefcon, void *commandRefcon,
                                     int32_t command, uint32_t priority,
                                     bool alreadyHandled, bool *handled);

int32_t registerCommandHook(int32_t pluginId, uint32_t priority,
                            int32_t command, commandHookFunc_t function,
                            void *functionData) {
    qInfo() << "registerCommandHook";
    QAction *action = aegpGlobalCommands[command];
    AEGP *plugin = AEGP::getPlugin(pluginId);
    action->connect(action, &QAction::triggered, action,
                    [function, functionData, plugin, command]() {
                        bool handled = false;
                        function(plugin->globalRefcon, functionData, command, 0,
                                 false, &handled);
                    });
    return 0;
}

int32_t registerUpdateMenuHook(int32_t pluginId, void *function,
                               void *functionData) {
    qInfo() << "registerUpdateMenuHook";
    return 0;
}

int32_t registerDeathHook(int32_t pluginId, void *function,
                          void *functionData) {
    qInfo() << "registerDeathHook";
    return 0;
}

int32_t registerVersionHook(int32_t pluginId, void *function,
                            void *functionData) {
    qInfo() << "registerVersionHook";
    return 0;
}

int32_t registerAboutStringHook(int32_t pluginId, void *function,
                                void *functionData) {
    qInfo() << "registerAboutStringHook";
    return 0;
}

int32_t registerAboutHook(int32_t pluginId, void *function,
                          void *functionData) {
    qInfo() << "registerAboutHook";
    return 0;
}

int32_t registerArtisan(AEVersion apiVersion, AEVersion artisanVersion,
                        int32_t pluginId, void *functionData, char *matchName,
                        char *artisanName, void *functions) {
    qInfo() << "registerArtisan";
    return 0;
}

int32_t registerIO(int32_t pluginId, void *functionData, void *moduleInfo,
                   void *functionBlock) {
    qInfo() << "registerIO";
    return 0;
}

int32_t registerIdleHook(int32_t pluginId, void *function, void *functionData) {
    qInfo() << "registerIdleHook";
    return 0;
}

int32_t registerTracker(AEVersion apiVersion, AEVersion trackerVersion,
                        int32_t pluginId, void *functionData, char *matchName,
                        char *trackerName, void *functions) {
    qInfo() << "registerTracker";
    return 0;
}

int32_t registerInteractiveArtisan(AEVersion apiVersion,
                                   AEVersion artisanVersion, int32_t pluginId,
                                   void *functionData, char *matchName,
                                   char *artisanName, void *functions) {
    qInfo() << "registerInteractiveArtisan";
    return 0;
}

AEGPRegisterSuite::AEGPRegisterSuite() {
    registerCommandHook = (void *)::registerCommandHook;
    registerUpdateMenuHook = (void *)::registerUpdateMenuHook;
    registerDeathHook = (void *)::registerDeathHook;
    registerVersionHook = (void *)::registerVersionHook;
    registerAboutStringHook = (void *)::registerAboutStringHook;
    registerAboutHook = (void *)::registerAboutHook;
    registerArtisan = (void *)::registerArtisan;
    registerIO = (void *)::registerIO;
    registerIdleHook = (void *)::registerIdleHook;
    registerTracker = (void *)::registerTracker;
    registerInteractiveArtisan = (void *)::registerInteractiveArtisan;
}
