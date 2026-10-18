#include "smart.hpp"
#include "animatable/effect/ae_plugin_effect.hpp"

int32_t getLayer(AEPluginEffectRender *effect, int32_t index,
                 int32_t loadedIndex, AERenderRequest *, int32_t, int32_t,
                 uint32_t, PrerenderLayer *layer) {
    qInfo() << "getLayer" << index << "to" << loadedIndex;
    SmartLoadedLayer *loadedLayer = new SmartLoadedLayer();
    loadedLayer->index = index;
    effect->smartLoadedLayers[loadedIndex] = loadedLayer;

    if (index == 0) {
        layer->originalWidth = effect->originalBox.w;
        layer->originalHeight = effect->originalBox.h;
        layer->rect = {0, 0, effect->lastBox.w, effect->lastBox.h};
        // TODO: what should actually be in maxRect?
        // layer->maxRect = {-300, -300, effect->lastBox.w + 300,
        //                   effect->lastBox.h + 300};
        layer->maxRect = layer->rect;
    } else {
        AELayer *theLayer = &effect->params[index]->layer;
        layer->originalWidth = theLayer->width;
        layer->originalHeight = theLayer->height;
        layer->rect = {0, 0, (int32_t)theLayer->width,
                       (int32_t)theLayer->height};
        layer->maxRect = layer->rect;
    }
    layer->pixelAspectRatio = {1, 1};
    return 0;
}

PrerenderFunctions::PrerenderFunctions() { getLayer = ::getLayer; }

PrerenderExtra::PrerenderExtra() {
    input = new PrerenderInput();
    output = new PrerenderOutput();
    functions = new PrerenderFunctions();
}

PrerenderExtra::~PrerenderExtra() {
    delete input;
    delete output;
    delete functions;
}

int32_t getLayerPixels(AEPluginEffectRender *effect, int32_t loadedIndex,
                       AELayer **toLayer) {
    qInfo() << "getLayerPixels";
    *toLayer = effect->smartLoadedLayers[loadedIndex]->layer;
    return 0;
}
int32_t freeLayerPixels(AEPluginEffectRender *effect, int32_t loadedIndex) {
    qInfo() << "freeLayerPixels";
    return 0;
}
int32_t getOutputPixels(AEPluginEffectRender *effect, AELayer **toLayer) {
    qInfo() << "getOutputPixels";
    *toLayer = effect->output;
    return 0;
}

SmartRenderFunctions::SmartRenderFunctions() {
    getLayerPixels = ::getLayerPixels;
    freeLayerPixels = ::freeLayerPixels;
    getOutputPixels = ::getOutputPixels;
}

SmartRenderExtra::SmartRenderExtra() {
    input = new SmartRenderInput();
    functions = new SmartRenderFunctions();
}
SmartRenderExtra::~SmartRenderExtra() {
    delete input;
    delete functions;
}
