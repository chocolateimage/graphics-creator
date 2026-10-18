#pragma once
#include "ae/ae_base.hpp"

struct AELayer;

enum PixelFormat {
    ARGB32 = 0x62677261,
};

struct PFWorldSuite2 {
    PFWorldSuite2();

    int32_t (*createWorld)(void *, int32_t, int32_t, bool, int32_t, AELayer *);
    int32_t (*deleteWorld)(void *, AELayer *);
    int (*getPixelFormat)(void *, PixelFormat *);
};
