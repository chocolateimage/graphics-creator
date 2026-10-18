#pragma once
#include "ae_base.hpp"
#include "layer.hpp"

class AEPluginEffectRender;

enum AEChannelMask {
    Alpha = 1 << 0,
    Red = 1 << 1,
    Green = 1 << 2,
    Blue = 1 << 3,
    Full = Alpha | Red | Green | Blue,
};

struct AERenderRequest {
    AERect rect;
    int32_t field;
    AEChannelMask mask;
    bool keepZeroRgb;
    char unused[19];
};

struct PrerenderLayer {
    AERect rect;
    AERect maxRect;
    Scale pixelAspectRatio;
    bool solid;
    bool reserved[3];
    int32_t originalWidth;
    int32_t originalHeight;
    int32_t reserved2[6];
};

struct PrerenderInput {
    AERenderRequest renderRequest;
    int16_t bitDepth;
    void *gpuData;
    int32_t gpu;
    uint32_t gpuIndex;
};

struct PrerenderOutput {
    AERect rect;
    AERect maxRect;
    bool solid;
    bool reserved;
    uint16_t flags;
    AEMemoryHandle *data;
    void *deleteDataFunc;
};

struct PrerenderFunctions {
    PrerenderFunctions();

    int32_t (*getLayer)(AEPluginEffectRender *, int32_t, int32_t,
                        AERenderRequest *, int32_t, int32_t, uint32_t,
                        PrerenderLayer *);
    void *func2{nullptr};
};

struct PrerenderExtra {
    PrerenderExtra();
    ~PrerenderExtra();

    PrerenderInput *input;
    PrerenderOutput *output;
    PrerenderFunctions *functions;
};

struct SmartRenderInput {
    AERenderRequest
        renderRequest; // to the effect: please only render this part
    int16_t bitDepth;
    AEMemoryHandle *data;
    void *gpuData;
    int32_t gpu;
    uint32_t gpuIndex;
};

struct SmartRenderFunctions {
    SmartRenderFunctions();
    int32_t (*getLayerPixels)(AEPluginEffectRender *, int32_t, AELayer **);
    int32_t (*freeLayerPixels)(AEPluginEffectRender *, int32_t);
    int32_t (*getOutputPixels)(AEPluginEffectRender *, AELayer **);
};

struct SmartRenderExtra {
    SmartRenderExtra();
    ~SmartRenderExtra();

    SmartRenderInput *input;
    SmartRenderFunctions *functions;
};

class SmartLoadedLayer {
  public:
    int index;
    AELayer *layer = nullptr;
};
