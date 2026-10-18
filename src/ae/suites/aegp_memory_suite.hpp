#pragma once
#include "ae/ae_base.hpp"

struct AEGPMemorySuite {
    AEGPMemorySuite();

    void *newMemoryHandle;
    void *freeMemoryHandle;
    void *lockMemoryHandle;
    void *unlockMemoryHandle;
    void *getMemoryHandleSize;
    void *resizeMemoryHandle;
    void *setMemoryReportingOn;
    void *getMemoryStats;
};
