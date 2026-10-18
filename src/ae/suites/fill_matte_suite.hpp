#pragma once
#include "ae/ae_base.hpp"

struct AELayer;

struct PFFillMatteSuite2 {
    PFFillMatteSuite2();

    int32_t (*fill)(void *, ARGB *, AERect *, AELayer *);
    int (*fill16)(void *, void *, void *, void *);
    int (*fillFloat)(void *, PixelFloat *, AERect *, AELayer *);
    int (*premultiply)(void *, int32_t, AELayer *);
    int (*premultiplyColor)(void *, void *, void *, int32_t, void *);
    int (*premultiplyColor16)(void *, void *, void *, int32_t, void *);
    int (*premultiplyColorFloat)(void *, void *, void *, int32_t, void *);
};
