#pragma once
#include "ae/ae_base.hpp"
#include "ae/layer.hpp"

struct PFWorldTransformSuite1 {
    PFWorldTransformSuite1();

    void *func1;
    int32_t (*blend)(void *, AELayer *, AELayer *, Fixed, AELayer *);
    void *func3;
    int32_t (*copy)(void *, AELayer *, AELayer *, AERect *, AERect *);
    int32_t (*copy2)(void *, AELayer *, AELayer *, AERect *, AERect *);
    void *transferRect;
    void *func7;
};
