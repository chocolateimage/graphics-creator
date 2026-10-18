#pragma once
#include "ae/ae_base.hpp"
#include "ae/utility_callbacks.hpp"

struct InData;
struct AELayer;

struct PFIterate8Suite1 {
    PFIterate8Suite1();

    int32_t (*iterate)(InData *, int32_t, int32_t, AELayer *, AERect *, void *,
                       iterateFunc_t, AELayer *);
    int32_t (*iterateOrigin)(InData *, int32_t, int32_t, AELayer *, AERect *,
                             Vector2DInt *, void *, iterateFunc_t, AELayer *);
};

struct PFIterate8Suite2 {
    PFIterate8Suite2();

    int32_t (*iterate)(InData *, int32_t, int32_t, AELayer *, AERect *, void *,
                       iterateFunc_t, AELayer *);
    int32_t (*iterateOrigin)(InData *, int32_t, int32_t, AELayer *, AERect *,
                             Vector2DInt *, void *, iterateFunc_t, AELayer *);
};

struct PFIterate16Suite2 {
    PFIterate16Suite2();

    void *func1;
    void *func2;
    void *func3;
};

struct PFIterateFloatSuite2 {
    PFIterateFloatSuite2();

    void *func1;
    void *func2;
    void *func3;
};
