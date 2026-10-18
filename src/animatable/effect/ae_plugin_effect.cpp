#include "ae_plugin_effect.hpp"
#include "ae/ae_base.hpp"
#include "ae/ae_data.hpp"
#include "ae/ae_effect.hpp"
#include "ae/ae_param.hpp"
#include "ae/layer.hpp"
#include "ae/smart.hpp"
#include "animatable/property.hpp"
#include "render.hpp"
#include "variant.hpp"
#include <cstddef>
#include <cstdint>

AEPluginEffectRender::AEPluginEffectRender(AEEffect *effect) : effect(effect) {
    for (AEEffectParameter *definition : effect->params) {
        PropertyRenderBase *property = nullptr;
        switch (definition->type) {
        case AEParameter::ParamType::FixedSlider:
        case AEParameter::ParamType::FloatSlider:
        case AEParameter::ParamType::Angle:
            property = new PropertyRender<double>(this);
            break;
        case AEParameter::ParamType::Color:
            property = new PropertyRender<Color>(this);
            break;
        case AEParameter::ParamType::Checkbox:
            property = new PropertyRender<bool>(this);
            break;
        case AEParameter::ParamType::Popup:
        case AEParameter::ParamType::Slider:
            property = new PropertyRender<int>(this);
            break;
        case AEParameter::ParamType::Point:
            property = new PropertyRender<Vector2DFloat>(this);
            break;
        case AEParameter::ParamType::Button:
            property = new PropertyRender<int>(this);
            break;
        case AEParameter::ParamType::Layer:
            property = new PropertyRender<ElementSelection>(this);
            break;
        case AEParameter::ParamType::NoData:
            property = new PropertyRender<bool>(this);
            break;
        case AEParameter::ParamType::Data:
            property = new PropertyRender<AEMemoryHandle *>(this);
            break;
        default:
            break;
        }

        if (!property) {
            pluginProperties.append(nullptr);
            continue;
        }

        pluginProperties.append(property);
    }
}

template <typename T>
Property<T> *AEPluginEffect::createProperty(AEEffectParameter *definition) {
    auto property = new Property<T>(this, definition->name.toStdString(),
                                    std::get<T>(definition->defaultValue));
    property->hasMin = true;
    property->hasMax = true;
    property->min = std::get<T>(definition->min);
    property->max = std::get<T>(definition->max);
    property->displayName = definition->name.toStdString();
    return property;
}

AEPluginEffect::AEPluginEffect(AEEffect *effect) : effect(effect) {
    for (AEEffectParameter *definition : effect->params) {
        PropertyBase *property = nullptr;
        switch (definition->type) {
        case AEParameter::ParamType::FixedSlider:
        case AEParameter::ParamType::FloatSlider:
        case AEParameter::ParamType::Angle:
            property = createProperty<double>(definition);
            break;
        case AEParameter::ParamType::Color:
            property = createProperty<Color>(definition);
            break;
        case AEParameter::ParamType::Checkbox:
            property = createProperty<bool>(definition);
            property->buttonText = definition->buttonText;
            break;
        case AEParameter::ParamType::Slider:
            property = createProperty<int>(definition);
            break;
        case AEParameter::ParamType::Point:
            property = createProperty<Vector2DFloat>(definition);
            break;
        case AEParameter::ParamType::Popup: {
            auto prop = createProperty<int>(definition);
            for (auto str : definition->choices) {
                prop->enumList.push_back(str.toStdString());
            }
            prop->updateBoundsToEnumList();
            property = prop;
            break;
        }
        case AEParameter::ParamType::Button:
            property = createProperty<int>(definition);
            property->isButton = true;
            property->buttonText = definition->buttonText;
            break;
        case AEParameter::ParamType::Layer:
            property = createProperty<ElementSelection>(definition);
            break;
        case AEParameter::ParamType::NoData:
            property = createProperty<bool>(definition);
            break;
        case AEParameter::ParamType::Data: {
            AEMemoryHandle *mem =
                std::get<AEMemoryHandle *>(definition->defaultValue);
            auto newProp = new Property<AEMemoryHandle *>(
                this, definition->name.toStdString(), mem->copy());
            newProp->hasMin = false;
            newProp->hasMax = false;
            newProp->displayName = definition->name.toStdString();
            property = newProp;
            break;
        }
        default:
            break;
        }

        if (!property)
            continue;

        property->aeParam = definition;

        pluginProperties.append(property);
    }

    InData *inData = new InData();
    OutData *outData = new OutData();
    inData->sequenceData = nullptr;
    outData->sequenceData = nullptr;
    inData->effect = this;
    qInfo() << "RUNNING SEQUENCE SETUP";
    effect->runCommand(AEEffect::Command::SequenceSetup, inData, outData,
                       nullptr, nullptr, nullptr);
    qInfo() << "RAN SEQUENCE SETUP";
    if (outData->sequenceData) {
        qInfo() << "SET UP SEQUENCE DATA";
        sequenceData = outData->sequenceData;
    }
    delete inData;
    delete outData;
}

AEPluginEffect::~AEPluginEffect() {
    qInfo() << "Delete AE effect";
    // InData *inData = new InData();
    // OutData *outData = new OutData();
    // effect->runCommand(AEEffect::Command::SequenceCleanup, inData, outData,
    //                    nullptr, nullptr, nullptr);
    // if (outData->sequenceData) {
    //     qInfo() << "SET UP SEQUENCE DATA";
    //     sequenceData = *outData->sequenceData;
    // }
    // delete inData;
    // delete outData;
    qDeleteAll(pluginProperties);
}

QString AEPluginEffect::getOptionsButtonText() {
    if (!effect->hasDialog) {
        return QString();
    } else if (effect->dialogButtonText != "") {
        return effect->dialogButtonText;
    } else {
        return "Options...";
    }
}

void AEPluginEffect::openOptions() {
    InData *inData = new InData();
    OutData *outData = new OutData();
    inData->effect = this;
    // TODO: params
    effect->runCommand(AEEffect::Command::DoDialog, inData, outData, nullptr,
                       nullptr, nullptr);
    delete inData;
    delete outData;
}

QString AEPluginEffect::effectName() { return "AE_" + effect->name; }

AnimatableRender *AEPluginEffect::toRender(const FrameInfo &frameInfo) {
    AEPluginEffectRender *render =
        (AEPluginEffectRender *)Effect::toRender(frameInfo);
    render->sequenceData = sequenceData;
    return render;
}

void AEPluginEffectRender::prepare() {
    // TODO: prepare calls before render box
    AEParameter *inputParam = new AEParameter();
    inputParam->type = AEParameter::ParamType::Layer;
    params.push_back(inputParam);

    int index = 0;
    for (auto def : effect->params) {
        AEParameter *param = new AEParameter();
        memset(param->data, 0, 120);
        param->type = def->type;
        PropertyRenderBase *propBase = pluginProperties[index];
        switch (def->type) {
        case AEParameter::ParamType::Popup: {
            param->popup.value = ((PropertyRender<int> *)propBase)->get() + 1;
            break;
        }
        case AEParameter::ParamType::FixedSlider: {
            param->fixedSlider.value =
                doubleToFixed(((PropertyRender<double> *)propBase)->get());
            break;
        }
        case AEParameter::ParamType::Angle: {
            param->angle.value =
                doubleToFixed(((PropertyRender<double> *)propBase)->get());
            break;
        }
        case AEParameter::ParamType::FloatSlider: {
            param->floatSlider.value =
                ((PropertyRender<double> *)propBase)->get();
            break;
        }
        case AEParameter::ParamType::Slider: {
            param->slider.value = ((PropertyRender<int> *)propBase)->get();
            break;
        }
        case AEParameter::ParamType::Checkbox: {
            param->checkbox.checked = ((PropertyRender<bool> *)propBase)->get();
            break;
        }
        case AEParameter::ParamType::Color: {
            Color &c = ((PropertyRender<Color> *)propBase)->get();
            param->color.value =
                ARGB{(uint8_t)c.a, (uint8_t)c.r, (uint8_t)c.g, (uint8_t)c.b};
            break;
        }
        case AEParameter::ParamType::Point: {
            Vector2DFloat &v =
                ((PropertyRender<Vector2DFloat> *)propBase)->get();
            param->point.x = doubleToFixed(v.x);
            param->point.y = doubleToFixed(v.y);
            break;
        }
        case AEParameter::ParamType::Layer: {
            if (renderThread) {
                ElementSelectionSnippet snippet = renderThread->getSnippet(
                    ((PropertyRender<ElementSelection> *)propBase)->get());

                if (snippet.values) {
                    AELayer *layer = &param->layer;
                    layer->stride = snippet.rect.w * 4;
                    layer->width = snippet.rect.w;
                    layer->height = snippet.rect.h;

                    uint32_t *copy = new uint32_t[layer->width * layer->height];
                    for (uint32_t i = 0; i < layer->width * layer->height;
                         i++) {
                        copy[i] = qbswap(snippet.values[i]);
                    }
                    copies.push_back(copy);
                    layer->data = (ARGB *)copy;
                    layer->rect.left = 0;
                    layer->rect.top = 0;
                    layer->rect.right = snippet.rect.w;
                    layer->rect.bottom = snippet.rect.h;
                    layer->pixelAspectRatio = {1, 1};
                }
            }
            break;
        }
        case AEParameter::ParamType::Data: {
            AEMemoryHandle *mem =
                ((PropertyRender<AEMemoryHandle *> *)propBase)->get();
            param->customData.value = mem;
            break;
        }
        default: {
            break;
        }
        }
        // if (param->type == AEParameter::Popup) {
        //     qInfo() << "============" << def->name << param->popup.value;
        // }
        // if (param->type == AEParameter::FixedSlider) {
        //     qInfo() << "=========================================";
        //     qInfo() << "ACTUAL" << ((PropertyRender<double>
        //     *)propBase)->get(); qInfo() << "AFTER FIXED" <<
        //     fixedToDouble(param->fixedSlider.value);
        // }
        params.push_back(param);
        index++;
    }
}

Rect AEPluginEffectRender::getRenderBox(const Rect &lastBox) {
    this->lastBox = lastBox;
    Rect renderBox = lastBox;
    prepare();
    // if (info->getRenderBoxFunc) {
    //     PluginEffectRenderContext ctx = getContext();
    //     info->getRenderBoxFunc(pluginInterface, &ctx, lastBox);
    //     return ctx.renderBox;
    // }
    inData = new InData();
    inData->effect = this;
    inData->sequenceData = sequenceData;
    inData->quality = 1;
    inData->preEffectOriginX = 0;
    inData->preEffectOriginY = 0;
    inData->originX = 0;
    inData->originY = 0;
    inData->timeScale = 1;
    inData->currentTime = currentFrame;
    inData->timeStep = frameRate;
    inData->totalTime = 100;
    inData->localTimeStep = 1;
    if (effect->isSmart) {
        PrerenderExtra *extra = new PrerenderExtra();
        OutData *outData = new OutData();
        extra->input->renderRequest.rect = {-300, -300, lastBox.w + 300,
                                            lastBox.h + 300};
        extra->output->data = nullptr;
        extra->output->rect = {0, 0, lastBox.w, lastBox.h};
        qInfo() << "[SMART] Original render box" << extra->output->rect.left
                << extra->output->rect.top << extra->output->rect.right
                << extra->output->rect.bottom;
        effect->runCommand(AEEffect::Command::SmartPrerender, inData, outData,
                           nullptr, nullptr, extra);
        renderBox.x += extra->output->rect.left;
        renderBox.y += extra->output->rect.top;
        renderBox.w = (extra->output->rect.right - extra->output->rect.left);
        renderBox.h = (extra->output->rect.bottom - extra->output->rect.top);
        smartData = extra->output->data;
        qInfo() << "[SMART] New render box" << extra->output->rect.left
                << extra->output->rect.top << extra->output->rect.right
                << extra->output->rect.bottom;
        if (outData->frameData) {
            frameData = outData->frameData;
        }
        delete outData;
        delete extra;
    } else {
        qInfo() << "Original render box" << lastBox.x << lastBox.y << lastBox.w
                << lastBox.h;
        OutData *outData = new OutData();
        outData->width = 0;
        outData->height = 0;
        outData->offset.x = 0;
        outData->offset.y = 0;
        effect->runCommand(AEEffect::Command::FrameSetup, inData, outData,
                           params.data(), nullptr, nullptr);
        renderBox.x -= outData->offset.x;
        renderBox.y -= outData->offset.y;
        renderBox.w += outData->width;
        renderBox.h += outData->height;
        qInfo() << "New render box" << renderBox.x << renderBox.y << renderBox.w
                << renderBox.h;
        if (outData->frameData) {
            frameData = outData->frameData;
        }
        delete outData;
    }
    return renderBox;
}

bool AEPluginEffectRender::render(const uint32_t *source,
                                  const Rect &sourceRect, uint32_t *target) {
    inData->frameData = frameData;

    OutData *outData = new OutData();
    AEParameter *inputParam = params[0];
    uint32_t *sourceCopy = new uint32_t[sourceRect.w * sourceRect.h];
    for (int i = 0; i < sourceRect.w * sourceRect.h; i++) {
        sourceCopy[i] = qbswap(source[i]);
    }
    inputParam->layer.data = (ARGB *)sourceCopy;
    inputParam->layer.width = sourceRect.w;
    inputParam->layer.height = sourceRect.h;
    inputParam->layer.stride = inputParam->layer.width * 4;
    inputParam->layer.rect.left = 0;
    inputParam->layer.rect.top = 0;
    inputParam->layer.rect.right = inputParam->layer.width;
    inputParam->layer.rect.bottom = inputParam->layer.height;
    inputParam->layer.pixelAspectRatio = {1, 1};

    inData->extentHint = inputParam->layer.rect;
    inData->width = originalBox.w;
    inData->height = originalBox.h;

    inData->preEffectOriginX = originalBox.x - sourceRect.x;
    inData->preEffectOriginY = originalBox.y - sourceRect.y;
    inData->originX = sourceRect.x - renderBox.x;
    inData->originY = sourceRect.y - renderBox.y;

    output = new AELayer();
    output->width = renderBox.w;
    output->height = renderBox.h;
    output->stride = output->width * 4;
    output->data = (ARGB *)target;
    output->rect.left = 0;
    output->rect.top = 0;
    output->rect.right = output->width;
    output->rect.bottom = output->height;
    output->originX = renderBox.x - sourceRect.x;
    output->originY = renderBox.y - sourceRect.y;
    output->pixelAspectRatio = {1, 1};

    // int index = 0;
    // for (auto param : effect->params) {
    //     if (param->type == AEParameter::ParamType::Layer) {
    //     }
    //     index++;
    // }

    for (auto pair : smartLoadedLayers) {
        SmartLoadedLayer *layer = pair.second;
        layer->layer = &params[layer->index]->layer;
    }

    int err;

    if (effect->isSmart) {
        SmartRenderExtra *smartRenderExtra = new SmartRenderExtra();
        smartRenderExtra->input->bitDepth = 8;
        smartRenderExtra->input->data = smartData;
        smartRenderExtra->input->renderRequest.rect = {0, 0, renderBox.w,
                                                       renderBox.h};
        err = effect->runCommand(AEEffect::Command::SmartRender, inData,
                                 outData, nullptr, nullptr, smartRenderExtra);
        delete smartRenderExtra;
    } else {
        err = effect->runCommand(AEEffect::Command::Render, inData, outData,
                                 params.data(), output, nullptr);
    }

    if (err != 0) {
        qCritical() << "FRAME RENDER ERROR" << err;
    }

    for (uint32_t i = 0; i < output->width * output->height; i++) {
        target[i] = qbswap(target[i]);
    }

    for (auto param : params) {
        delete param;
    }

    delete outData;
    delete output;
    delete[] sourceCopy;

    for (auto copy : copies) {
        delete[] copy;
    }

    return true;
}

AEPluginEffectRender::~AEPluginEffectRender() {
    for (auto prop : pluginProperties) {
        if (prop) {
            delete prop;
        }
    }

    for (auto layer : smartLoadedLayers) {
        delete layer.second;
    }

    if (smartData) {
        delete smartData;
    }

    if (inData) {
        delete inData;
    }
}
