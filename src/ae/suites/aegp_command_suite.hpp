#pragma once
#include "ae/ae_base.hpp"

class QAction;

extern QList<QAction *> aegpGlobalCommands;

struct AEGPCommandSuite1 {
    enum Menu {
        None,
        Apple,
        File,
        Edit,
        Composition,
        Layer,
        Effect,
        Window,
        Floaters,
        KeyframeAssist,
        Import,
        SaveFrameAs,
        Prefs,
        Export,
        Animation,
        Purge,
        New
    };

    AEGPCommandSuite1();

    void *getUniqueCommand;
    void *insertMenuCommand;
    void *removeMenuCommand;
    void *setMenuCommandName;
    void *enableCommand;
    void *disableCommand;
    void *checkmarkMenuCommand;
    void *doCommand;
};
