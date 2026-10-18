#include "interaction_callbacks.hpp"
#include "ae/ae_base.hpp"
#include "ae_effect.hpp"
#include "ae_param.hpp"
#include "animatable/effect/ae_plugin_effect.hpp"
#include "variant.hpp"

int inquireParam(AEPluginEffectRender *effect, int64_t index, int64_t, int64_t,
                 uint64_t, AEParameter *parameter) {
    // qInfo() << "inquireParam() called";
    // qInfo() << "index" << index;
    // qInfo() << "effect" << ((void *)effect);
    // qInfo() << "params" << effect->params.size();
    *parameter = *effect->params[index];
    // if (index == 1) {
    //     parameter->fixedSlider.value.fractional = 0;
    //     parameter->fixedSlider.value.number = 100;
    // }
    // if (index == 2) {
    //     parameter->checkbox.checked = true;
    // }
    // if (index == 3) {
    //     parameter->checkbox.checked = false;
    // }
    // *parameter = *effect->params[index];
    // abort();
    return 0;
}
int inquireParamClose(void *, AEParameter *) {
    // qInfo() << "inquireParamClose() called";
    return 0;
}

#define ADDPARAM_DEBUG
#undef ADDPARAM_DEBUG

int addParam(AEEffect *effect, int index, AEParameter *definition) {
#ifdef ADDPARAM_DEBUG
    qInfo() << "addParam";
    qInfo() << "effect" << effect;
    qInfo() << "index" << index;
    qInfo() << "definition name" << definition->name;
    qInfo() << "definition type" << definition->type;
#endif
    AEEffectParameter *param = new AEEffectParameter();
    param->effect = effect;
    param->name = definition->name;
    param->type = definition->type;
    param->uiFlags = definition->uiFlags;
    param->uiWidth = definition->uiWidth;
    param->uiHeight = definition->uiHeight;
    switch (param->type) {
    case AEParameter::ParamType::Popup: {
#ifdef ADDPARAM_DEBUG
        qInfo() << "popup info";
        qInfo() << "choiceCount" << definition->popup.choiceCount;
        qInfo() << "defaultValue" << definition->popup.defaultValue;
        qInfo() << "value" << definition->popup.value;
        qInfo() << "names" << definition->popup.names;
#endif
        param->min = 0;
        param->max = definition->popup.choiceCount - 1;
        param->defaultValue = definition->popup.defaultValue - 1;
        QString names(definition->popup.names);
        param->choices = names.split('|');
        break;
    }
    case AEParameter::ParamType::FixedSlider: {
#ifdef ADDPARAM_DEBUG
        qInfo() << "--- FixedSlider info ---";
        qInfo() << "min.number" << definition->fixedSlider.min.number;
        qInfo() << "min.fractional" << definition->fixedSlider.min.fractional;
        qInfo() << "max.number" << definition->fixedSlider.max.number;
        qInfo() << "max.fractional" << definition->fixedSlider.max.fractional;
        qInfo() << "decimalPlaces" << definition->fixedSlider.decimalPlaces;
        qInfo() << "str" << definition->fixedSlider.str;
        qInfo() << "description" << definition->fixedSlider.description;
#endif
        param->min = fixedToDouble(definition->fixedSlider.min);
        param->max = fixedToDouble(definition->fixedSlider.max);
        param->defaultValue =
            fixedToDouble(definition->fixedSlider.defaultValue);
        break;
    }
    case AEParameter::ParamType::FloatSlider: {
#ifdef ADDPARAM_DEBUG
        qInfo() << "--- FloatSlider info ---";
        qInfo() << "min" << definition->floatSlider.min;
        qInfo() << "max" << definition->floatSlider.max;
        qInfo() << "MIN" << definition->floatSlider.min2;
        qInfo() << "MAX" << definition->floatSlider.max2;
#endif
        param->min = (double)definition->floatSlider.min;
        param->max = (double)definition->floatSlider.max;
        param->defaultValue = (double)definition->floatSlider.defaultValue;
        break;
    };
    case AEParameter::ParamType::Color: {
        param->min = Color{0, 0, 0, 0};
        param->max = Color{0, 0, 0, 255};
        param->defaultValue = Color{
            definition->color.defaultValue.r, definition->color.defaultValue.g,
            definition->color.defaultValue.b, definition->color.defaultValue.a};
        break;
    };
    case AEParameter::ParamType::Checkbox: {
        param->min = false;
        param->max = true;
        param->defaultValue = definition->checkbox.defaultValue;
        param->buttonText = definition->checkbox.name;
        break;
    };
    case AEParameter::ParamType::Angle: {
        param->min = -9999999.f;
        param->max = 9999999.f;
        param->defaultValue =
            (float)fixedToDouble(definition->angle.defaultValue);
        break;
    };
    case AEParameter::ParamType::Point: {
        param->min = Vector2DFloat{-999999, -999999};
        param->max = Vector2DFloat{999999, 999999};
        param->defaultValue =
            Vector2DFloat{(float)fixedToDouble(definition->point.defaultX),
                          (float)fixedToDouble(definition->point.defaultY)};
        break;
    };
    case AEParameter::ParamType::Slider: {
        param->min = definition->slider.min;
        param->max = definition->slider.max;
        param->defaultValue = definition->slider.defaultValue;
        break;
    };
    case AEParameter::ParamType::GroupStart:
    case AEParameter::ParamType::GroupEnd: {
        break;
    };
    case AEParameter::ParamType::Button: {
        param->min = 0;
        param->max = 0;
        param->defaultValue = 0;
        param->buttonText = definition->button.text;
        break;
    };
    case AEParameter::ParamType::Layer: {
        param->min = ElementSelection();
        param->max = ElementSelection();
        param->defaultValue = ElementSelection();
        break;
    };
    case AEParameter::ParamType::NoData: {
        param->min = false;
        param->max = false;
        param->defaultValue = false;
        break;
    };
    case AEParameter::ParamType::Data: {
        param->min = (AEMemoryHandle *)0;
        param->max = (AEMemoryHandle *)0xFFFFFFFFFFFFFFFF;
        param->defaultValue = definition->customData.defaultValue;
        break;
    };
    default: {
        qInfo() << "UNKNOWN PARAM TYPE" << param->type;
        abort();
        break;
    }
    }
    effect->params.push_back(param);
    return 0;
}

int registerUi(AEEffect *effect, AECustomUIInfo *info) {
    qInfo() << "registerUI()";
    effect->registeredCustomUi = true;
    return 0;
}

struct AEInternalStrings2 {
    char *lastElement;
    uint8_t count;
};

uint32_t initData(char ***arg1, uint32_t value) {
    // qInfo() << "reserved1/initData";
    AEInternalStrings2 *strings2 = (AEInternalStrings2 *)&(**arg1);
    // char argu1 = (*arg1)[1];
    // char *argu2 = (*arg1)[2];
    // qInfo() << "effectName" << effectName;
    // qInfo() << "value" << value;
    AEInternalStrings *strings = new AEInternalStrings();
    int i = 0;
    int elementIndex = 1;
    while (elementIndex < strings2->count) {
        char *elementStr = (strings2->lastElement - i);
        if (elementStr[0] == '$' && elementStr[1] == '$' &&
            elementStr[2] == '$') {
            elementIndex++;
            char *str = strchr(elementStr, '=') + 1;
            strings->strings.insert(strings->strings.begin(), std::string(str));
        }
        i++;
    }
    // qInfo() << "argu1" << arg1;
    // qInfo() << "argu2" << argu2;
    // AEInternalStrings *strings = new AEInternalStrings();
    // strings->effectName = effectName;
    // strings->version = value;
    // qInfo() << "STRINGS IS" << strings;

    *arg1 = (char **)strings;
    return 0;
}

char *getString(void *globalData, uint8_t a, uint8_t b, uint8_t c) {
    // qInfo() << "reserved2/getString";
    AEInternalStrings *strings = (AEInternalStrings *)globalData;
    // qInfo() << "globalData IS" << globalData;
    // qInfo() << "ARG 1:" << a;
    // qInfo() << "ARG 2:" << b;
    // qInfo() << "ARG 3:" << c;
    // qInfo() << "strings available" << strings->strings.size();
    std::string &str = strings->strings[a];
    // qInfo() << "STRING IS" << str;
    return (char *)str.c_str();
}

void cleanupData(void *globalData) {
    // qInfo() << "reserved3/cleanupData";
    AEInternalStrings *strings = (AEInternalStrings *)globalData;
    delete strings;
}

int abort(void *) {
    // qInfo() << "abort()";
    return 0;
}
int progress(void *, int32_t a, int32_t b) { return 0; }

InteractionCallbacks::InteractionCallbacks() {
    inquireParam = ::inquireParam;
    inquireParamClose = ::inquireParamClose;
    addParam = ::addParam;
    abort = ::abort;
    progress = ::progress;
    registerUi = ::registerUi;
    initData = ::initData;
    getString = ::getString;
    cleanupData = ::cleanupData;
}
