#pragma once
#include "ae/ae_base.hpp"

struct PFHandleSuite2 {
    PFHandleSuite2();

    AEMemoryHandle *(*createMemory)(size_t size);
    void *(*useMemory)(AEMemoryHandle *handle);
    void (*freeMemoryUsage)(AEMemoryHandle *handle);
    void (*freeMemory)(AEMemoryHandle *handle);
    void *getMemorySize;
    void *resizeMemory{nullptr};
};
