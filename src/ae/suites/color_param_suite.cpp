#include "color_param_suite.hpp"
#include "ae/ae_base.hpp"
#include "ae/ae_param.hpp"

int32_t getFloatingPointColorFromColorDef(void *, AEParameter *param,
                                          PixelFloat *color) {
    color->a = param->color.value.a / 255.;
    color->r = param->color.value.r / 255.;
    color->g = param->color.value.g / 255.;
    color->b = param->color.value.b / 255.;
    return 0;
}

PFColorParamSuite1::PFColorParamSuite1() {
    getFloatingPointColorFromColorDef =
        (void *)::getFloatingPointColorFromColorDef;
}
