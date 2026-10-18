#pragma once
#include "ae/ae_base.hpp"

struct AEGPUtilitySuite3 {
    AEGPUtilitySuite3();

    void *reportInfo;
    void *getPluginInitFuncVersion;
    void *getImplementationVersion;
    void *startQuietErrors;
    void *endQuietErrors;
    void *startUndoGroup;
    void *endUndoGroup;
    void *registerWithAegp;
};

struct AEGPUtilitySuite6 {
    AEGPUtilitySuite6();

    void *reportInfo;
    void *reportInfoUnicode;
    void *getPluginInitFuncVersion;
    void *getImplementationVersion;
    void *startQuietErrors;
    void *endQuietErrors;
    void *getLastErrorMessage;
    void *startUndoGroup;
    void *endUndoGroup;
    void *registerWithAegp;
};
