#pragma once
#include "ae/ae_base.hpp"

struct PFParamUtilsSuite3 {
    PFParamUtilsSuite3();

    void *updateParamUI;
    void *getCurrentState;
    void *areStatesIdentical;
    void *isIdenticalCheckout;
    void *findKeyframeTime;
    void *getKeyframeCount;
    void *checkoutKeyframe;
    void *checkinKeyframe;
    void *keyIndexToTime;
};
