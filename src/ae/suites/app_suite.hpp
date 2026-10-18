#pragma once
#include "ae/ae_base.hpp"

struct PFAppSuite6 {
    PFAppSuite6();

    void *appGetBgColor;
    void *appGetColor;
    void *getPersonalInfo;
    void *getFontStylesheet;
    void *setCursor;
    void *isRenderEngine;
    void *showColorPicker;
    void *getMouse;
    void *invaldiateRect;
    void *convertLocalToGlobal;
    void *getColorAtGlobalPoint;
};

struct PFAppSuite7 {
    PFAppSuite7();

    void *appGetBgColor;
    void *appGetColor;
    void *appGetLanguage;
    void *getPersonalInfo;
    void *getFontStylesheet;
    void *setCursor;
    void *isRenderEngine;
    void *showColorPicker;
};

struct PFAdvAppSuite2 {
    PFAdvAppSuite2();

    void *setProjectDirty;
    void *saveProject;
    void *saveBackgroundState;
    void *forceForeground;
    void *restoreBackgroundState;
    void *refreshAllWindows;
    void *infoDrawText2Lines;
    void *infoDrawColor;
    void *infoDrawText3Lines;
    void *infoDrawText3LinesPlus;
    void *appendInfoText;
};
