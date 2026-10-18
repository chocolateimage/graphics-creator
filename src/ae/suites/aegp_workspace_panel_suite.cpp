#include "aegp_workspace_panel_suite.hpp"

int32_t registerCreatePanelHook(int32_t pluginId, char *matchName, void *,
                                void *, bool) {
    qInfo() << "registerCreatePanelHook with name" << matchName;
    return 0;
}
int32_t unregisterCreatePanelHook(char *matchName) {
    qInfo() << "unregisterCreatePanelHook name" << matchName;
    return 0;
}
int32_t setTitle(void *panel, char *name) {
    qInfo() << "setTitle to" << name;
    return 0;
}
int32_t toggleVisibility(char *matchName) {
    qInfo() << "toggleVisibility" << matchName;
    return 0;
}
int32_t isShown(char *matchName, bool *isShown, bool *isFrontmost) {
    qInfo() << "isShown" << matchName;
    *isShown = false;
    *isFrontmost = false;
    return 0;
}

AEGPWorkspacePanelSuite1::AEGPWorkspacePanelSuite1() {
    registerCreatePanelHook = (void *)::registerCreatePanelHook;
    unregisterCreatePanelHook = (void *)::unregisterCreatePanelHook;
    setTitle = (void *)::setTitle;
    toggleVisibility = (void *)::toggleVisibility;
    isShown = (void *)::isShown;
}
