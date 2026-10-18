#pragma once
#include "ae/ae_base.hpp"

struct AEGPRegisterSuite {
    AEGPRegisterSuite();

    void *registerCommandHook;
    void *registerUpdateMenuHook;
    void *registerDeathHook;
    void *registerVersionHook;
    void *registerAboutStringHook;
    void *registerAboutHook;
    void *registerArtisan;
    void *registerIO;
    void *registerIdleHook;
    void *registerTracker;
    void *registerInteractiveArtisan;
};
