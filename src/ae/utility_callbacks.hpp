#pragma once
#include "../variant.hpp"
#include "ae_base.hpp"
#include "ae_data.hpp"
#include "layer.hpp"

typedef int32_t (*iterateFunc_t)(void *data, int32_t x, int32_t y,
                                 ARGB *inPixel, ARGB *outPixel);

int32_t blend(void *, AELayer *a, AELayer *b, Fixed ratio, AELayer *dest);
int32_t copy(void *, AELayer *, AELayer *, AERect *, AERect *);
int32_t fill(void *effect, ARGB *color, AERect *rect, AELayer *layer);
int fill16(void *, void *, void *, void *);
int fillFloat(void *, PixelFloat *color, AERect *rect, AELayer *layer);
int premultiply(void *, int32_t, AELayer *);
int premultiplyColor(void *, void *, void *, int32_t, void *);
int premultiplyColor16(void *, void *, void *, int32_t, void *);
int premultiplyColorFloat(void *, void *, void *, int32_t, void *);
int32_t iterate(InData *, int32_t, int32_t, AELayer *, AERect *, void *,
                iterateFunc_t, AELayer *);
int32_t iterateOrigin(InData *, int32_t, int32_t, AELayer *, AERect *,
                      Vector2DInt *, void *, iterateFunc_t, AELayer *);
AEMemoryHandle *createMemory(size_t size);
void *useMemory(AEMemoryHandle *handle);
void freeMemoryUsage(AEMemoryHandle *handle);
void freeMemory(AEMemoryHandle *handle);
int32_t createWorld(void *, int32_t, int32_t, int32_t, AELayer *);
int32_t deleteWorld(void *, AELayer *);
int32_t transferRect(void *, int32_t, int32_t, int32_t, AERect *, AELayer *,
                     AECompositeMode *, void *, int32_t, int32_t, AELayer *);

struct SubpixelSampleData {
    Fixed unused1;
    Fixed unused2;
    Fixed unused3;
    AELayer *source;
};

struct UtilityCallbacks {
    UtilityCallbacks();

    void *func1{nullptr};
    int32_t (*subpixelSample)(void *, Fixed x, Fixed y, SubpixelSampleData *,
                              ARGB *);
    void *func3{nullptr};
    void *func4{nullptr};
    void *func5{nullptr};
    void *func6{nullptr};
    int32_t (*blend)(void *, AELayer *, AELayer *, Fixed, AELayer *);
    void *func8{nullptr};
    int32_t (*copy)(void *, AELayer *, AELayer *, AERect *, AERect *);
    int32_t (*fill)(void *, ARGB *, AERect *, AELayer *);
    void *func11{nullptr};
    int32_t (*iterate)(InData *, int32_t, int32_t, AELayer *, AERect *, void *,
                       iterateFunc_t, AELayer *);
    int32_t (*premultiply)(void *, int32_t, AELayer *);
    void *func14{nullptr};
    int32_t (*createWorld)(void *, int32_t, int32_t, int32_t, AELayer *);
    int32_t (*deleteWorld)(void *, AELayer *);
    int32_t (*iterateOrigin)(InData *, int32_t, int32_t, AELayer *, AERect *,
                             Vector2DInt *, void *, iterateFunc_t, AELayer *);
    void *func18{nullptr};
    int32_t (*transferRect)(void *, int32_t, int32_t, int32_t, AERect *,
                            AELayer *, AECompositeMode *, void *, int32_t,
                            int32_t, AELayer *);
    void *func20{nullptr};
    AEMemoryHandle *(*createMemory)(size_t size);
    void *(*useMemory)(AEMemoryHandle *handle);
    void (*freeMemoryUsage)(AEMemoryHandle *handle);
    void (*freeMemory)(AEMemoryHandle *handle);
    int32_t (*getCallbackAddress)(void *, int32_t, int32_t, int32_t, void **);
    void *__func5{nullptr};
    double (*atan)(double);
    double (*atan2)(double, double);
    double (*ceil)(double);
    double (*cos)(double);
    double (*exp)(double);
    double (*fabs)(double);
    double (*floor)(double);
    double (*fmod)(double, double);
    double (*hypot)(double, double);
    double (*log)(double);
    double (*log10)(double);
    double (*pow)(double, double);
    double (*sin)(double);
    double (*sqrt)(double);
    double (*tan)(double);
    int (*sprintf)(char *, const char *, ...);
    char *(*strcpy)(char *, const char *);
    double (*asin)(double);
    double (*acos)(double);
    void *unused{nullptr};
    int32_t (*rgbToHls)(void *, ARGB *, int32_t[3]);
    int32_t (*hlsToRgb)(void *, int32_t[3], ARGB *);
    int32_t (*rgbToYiq)(void *, ARGB *, int32_t[3]);
    int32_t (*yiqToRgb)(void *, int32_t[3], ARGB *);
    int32_t (*luminance)(void *, ARGB *, int32_t *);
    int32_t (*hue)(void *, ARGB *, int32_t[3], int32_t *);
    int32_t (*lightness)(void *, ARGB *, int32_t *);
    int32_t (*saturation)(void *, ARGB *, int32_t *);
    void *getPlatformData;
    void *getMemorySize;
    void *iterateOriginNonClipSrc;
    int32_t (*iterateGeneric)(int32_t, void *, void *);
    void *resizeMemory{nullptr};
    void *subpixelSample16{nullptr};
    void *areaSample16{nullptr};
    void *fill16{nullptr};
    void *premultiplyColor16{nullptr};
    int32_t (*iterate16)(InData *, int32_t, int32_t, AELayer *, AERect *,
                         void *, void *, AELayer *);
    int32_t (*iterateOrigin16)(InData *, int32_t, int32_t, AELayer *, AERect *,
                               Vector2DInt *, void *, void *, AELayer *);
    void *iterateOriginNonClipSrc16{nullptr};
    void *getPixelData8{nullptr};
    void *getPixelData16{nullptr};
    void *unused2{nullptr};
};
