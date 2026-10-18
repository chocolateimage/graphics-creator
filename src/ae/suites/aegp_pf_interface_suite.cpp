#include "aegp_pf_interface_suite.hpp"
#include "ae/ae_base.hpp"

int32_t getEffectLayer(void *, void *) {
    qInfo() << "getEffectLayer";

    return 0;
}

int32_t getNewEffectForEffect(int32_t, void *, void *) {
    qInfo() << "getNewEffectForEffect";

    return 0;
}

int32_t convertEffectToCompTime(void *, int32_t, uint32_t, void *) {
    qInfo() << "convertEffectToCompTime";

    return 0;
}

int32_t getEffectCamera(void *, void *, void *) {
    qInfo() << "getEffectCamera";

    return 0;
}

int32_t getEffectCameraMatrix(void *, void *, void *, void *, void *, void *) {
    qInfo() << "getEffectCameraMatrix";

    return 0;
}

AEGPPFInterfaceSuite1::AEGPPFInterfaceSuite1() {
    getEffectLayer = (void *)::getEffectLayer;
    getNewEffectForEffect = (void *)::getNewEffectForEffect;
    convertEffectToCompTime = (void *)::convertEffectToCompTime;
    getEffectCamera = (void *)::getEffectCamera;
    getEffectCameraMatrix = (void *)::getEffectCameraMatrix;
}
