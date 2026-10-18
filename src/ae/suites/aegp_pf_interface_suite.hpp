#pragma once
#include "ae/ae_base.hpp"

struct AEGPPFInterfaceSuite1 {
    AEGPPFInterfaceSuite1();

    void *getEffectLayer;
    void *getNewEffectForEffect;
    void *convertEffectToCompTime;
    void *getEffectCamera;
    void *getEffectCameraMatrix;
};
