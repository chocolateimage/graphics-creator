#pragma once
#include <QtSystemDetection>
#ifdef Q_OS_WIN
#include "ae/ae_base.hpp"
#include <windows.h>

class AEPanelWindow;
class AEGP;

typedef int32_t (*createPanelFunc_t)(void *globalRefcon, void *funcData,
                                     HWND hwnd, AEPanelWindow *panel,
                                     void *functions, void **refcon);

class AEGPPanelRegistration {
  public:
    AEGP *aegp;
    AEPanelWindow *panelWidget{nullptr};
    createPanelFunc_t createPanelFunc;
    void *funcData;
    QString matchName;
};

extern QMap<QString, AEGPPanelRegistration *> registeredPanels;

struct AEGPWorkspacePanelSuite1 {
    AEGPWorkspacePanelSuite1();

    void *registerCreatePanelHook;
    void *unregisterCreatePanelHook;
    void *setTitle;
    void *toggleVisibility;
    void *isShown;
};
#else
struct AEGPWorkspacePanelSuite1 {};
#endif
