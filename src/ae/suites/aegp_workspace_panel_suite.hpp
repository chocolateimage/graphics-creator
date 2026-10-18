#pragma once
#include "ae/ae_base.hpp"

struct AEGPWorkspacePanelSuite1 {
    AEGPWorkspacePanelSuite1();

    void *registerCreatePanelHook;
    void *unregisterCreatePanelHook;
    void *setTitle;
    void *toggleVisibility;
    void *isShown;
};
