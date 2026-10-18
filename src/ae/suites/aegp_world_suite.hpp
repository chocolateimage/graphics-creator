#pragma once
#include "ae/ae_base.hpp"

struct AEGPWorldSuite3 {
    AEGPWorldSuite3();

    void *create;
    void *dispose;
    void *getType;
    void *getSize;
    void *getRowBytes;
    void *getBaseAddress8;
    void *getBaseAddress16;
    void *getBaseAddress32;
    void *fillOutWorld;
    void *fastBlur;
    void *createPlatformWorld;
    void *disposePlatformWorld;
    void *newReferenceFromPlatformWorld;
};
