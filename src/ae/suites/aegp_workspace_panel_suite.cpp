#include "aegp_workspace_panel_suite.hpp"
#ifdef Q_OS_WIN
#include "ae/aegp.hpp"
#include "gui/ae_panel.hpp"

QMap<QString, AEGPPanelRegistration *> registeredPanels{};

int32_t registerCreatePanelHook(int32_t pluginId, char *matchName,
                                createPanelFunc_t func, void *funcData, bool) {
    qInfo() << "registerCreatePanelHook with name" << matchName;
    QString name = matchName;
    AEGPPanelRegistration *registration = new AEGPPanelRegistration();
    registration->aegp = AEGP::getPlugin(pluginId);
    registration->createPanelFunc = func;
    registration->funcData = funcData;
    registration->matchName = matchName;
    registeredPanels[name] = registration;
    return 0;
}
int32_t unregisterCreatePanelHook(char *matchName) {
    qInfo() << "unregisterCreatePanelHook name" << matchName;
    return 0;
}
int32_t setTitle(AEPanelWindow *panel, char *name) {
    qInfo() << "setTitle to" << name;
    panel->setWindowTitle(name);
    return 0;
}
int32_t toggleVisibility(char *matchName) {
    qInfo() << "toggleVisibility" << matchName;
    QString name = matchName;
    AEGPPanelRegistration *registration = registeredPanels[name];

    // TODO: or hidden
    if (!registration->panelWidget) {
        registration->panelWidget = new AEPanelWindow(registration);
        registration->panelWidget->show();
    }
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
#endif
