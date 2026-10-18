#pragma once
#include "ae_base.hpp"

struct SuiteLoader {
    SuiteLoader();

    uint32_t (*loadSuite)(const char *name, int32_t version,
                          const void **suite);

    uint32_t (*unloadSuite)(const char *name, int32_t version);
    bool (*isEqual)(const char *token1, const char *token2);
    uint32_t (*allocateBlock)(size_t size, void **block);
    uint32_t (*freeBlock)(void *block);
    uint32_t (*reallocateBlock)(void *block, size_t newSize, void **newblock);
};
