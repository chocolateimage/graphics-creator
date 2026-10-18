#include "effect_ui_suite.hpp"
#include "ae/ae_effect.hpp"
#include "ae/ae_param.hpp"

int32_t setOptionsButtonName(AEEffect *effect, char *name) {
    effect->dialogButtonText = name;
    return 0;
}

PFEffectUISuite::PFEffectUISuite() {
    setOptionsButtonName = (void *)::setOptionsButtonName;
}

int32_t getDrawingReference(AEContext **context, void **drawRef) {
    *drawRef = (*context)->drawRef;
    return 0;
}

PFEffectCustomUISuite1::PFEffectCustomUISuite1() {
    getDrawingReference = (void *)::getDrawingReference;
}
