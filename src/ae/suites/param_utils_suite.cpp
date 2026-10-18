#include "param_utils_suite.hpp"
#include "ae/ae_base.hpp"
#include "ae/ae_param.hpp"

int32_t updateParamUI(void *effect, int32_t paramIndex, AEParameter *param) {
    qInfo() << "updateParamUI";
    return 0;
}
int32_t getCurrentState(void *effect, int32_t paramIndex, AETime *start,
                        AETime *duration, void *state) {
    qInfo() << "getCurrentState";
    return 0;
}
int32_t areStatesIdentical(void *effect, void *state1, void *state2,
                           bool *same) {
    qInfo() << "areStatesIdentical";
    return 0;
}
int32_t isIdenticalCheckout(void *effect, int32_t paramIndex, int32_t whatTime1,
                            int32_t timeStep1, uint32_t timeScale1,
                            int32_t whatTime2, int32_t timeStep2,
                            uint32_t timeScale2, bool *same) {
    qInfo() << "isIdenticalCheckout";
    return 0;
}
int32_t findKeyframeTime(void *effect, int32_t paramIndex, int32_t whatTime,
                         uint32_t timeScale, int32_t timeDir, bool *found,
                         int32_t *keyIndex, int32_t *keyTime,
                         uint32_t *keyTimescale) {
    qInfo() << "findKeyframeTime";
    return 0;
}
int32_t getKeyframeCount(void *effect, int32_t paramIndex, int32_t *keyCount) {
    qInfo() << "getKeyframeCount";
    return 0;
}
int32_t checkoutKeyframe(void *effect, int32_t paramIndex, int32_t keyIndex,
                         int32_t *keyTime, uint32_t *keyTimescale,
                         AEParameter *param) {
    qInfo() << "checkoutKeyframe";
    return 0;
}
int32_t checkinKeyframe(void *effect, AEParameter *param) {
    qInfo() << "checkinKeyframe";
    return 0;
}
int32_t keyIndexToTime(void *effect, int32_t paramIndex, int32_t keyIndex,
                       int32_t *keyTime, uint32_t *keyTimescale) {
    qInfo() << "keyIndexToTime";
    return 0;
}

PFParamUtilsSuite3::PFParamUtilsSuite3() {
    updateParamUI = (void *)::updateParamUI;
    getCurrentState = (void *)::getCurrentState;
    areStatesIdentical = (void *)::areStatesIdentical;
    isIdenticalCheckout = (void *)::isIdenticalCheckout;
    findKeyframeTime = (void *)::findKeyframeTime;
    getKeyframeCount = (void *)::getKeyframeCount;
    checkoutKeyframe = (void *)::checkoutKeyframe;
    checkinKeyframe = (void *)::checkinKeyframe;
    keyIndexToTime = (void *)::keyIndexToTime;
}
