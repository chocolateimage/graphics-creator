#include "utility_callbacks.hpp"
#include "ae/ae_base.hpp"
#include "layer.hpp"
#include "math.hpp"
#include <cmath>
#include <cstring>

extern "C" {
#include <libswscale/swscale.h>
}

int32_t fill(void *effect, ARGB *color, AERect *rect, AELayer *layer) {
    qInfo() << "fill() CALLED";

    ARGB finalColor = {0, 0, 0, 0};
    if (color != nullptr) {
        finalColor = *color;
    }

    int x = 0;
    int y = 0;
    int w = layer->width;
    int h = layer->height;
    if (rect) {
        qInfo() << "has rect";
        qInfo() << "left" << rect->left;
        qInfo() << "top" << rect->top;
        qInfo() << "right" << rect->right;
        qInfo() << "bottom" << rect->bottom;
        x = rect->left;
        y = rect->top;
        w = rect->right - rect->left;
        h = rect->bottom - rect->top;
    }

    for (int y_ = 0; y_ < h; y_++) {
        for (int x_ = 0; x_ < w; x_++) {
            *(ARGB *)&layer->data[layer->width * (y_ + y) + x_ + x] =
                finalColor;
        }
    }

    return 0;
}

int fill16(void *, void *, void *, void *) {
    qInfo() << "fill16";
    abort();
    return 0;
}

int fillFloat(void *, PixelFloat *color, AERect *rect, AELayer *layer) {
    qInfo() << "fillMatteSuite_fillFloat";
    qInfo() << "layer" << layer;
    qInfo() << "r" << color->r;
    qInfo() << "g" << color->g;
    qInfo() << "b" << color->b;
    qInfo() << "a" << color->a;
    ARGB finalColor = {(uint8_t)(color->a * 255), (uint8_t)(color->r * 255),
                       (uint8_t)(color->g * 255), (uint8_t)(color->b * 255)};
    int x = 0;
    int y = 0;
    int w = layer->width;
    int h = layer->height;
    if (rect) {
        qInfo() << "has rect";
        qInfo() << "left" << rect->left;
        qInfo() << "top" << rect->top;
        qInfo() << "right" << rect->right;
        qInfo() << "bottom" << rect->bottom;
        x = rect->left;
        y = rect->top;
        w = rect->right - rect->left;
        h = rect->bottom - rect->top;
    }

    for (int y_ = 0; y_ < h; y_++) {
        for (int x_ = 0; x_ < w; x_++) {
            *(ARGB *)&layer->data[layer->width * (y_ + y) + x_ + x] =
                finalColor;
        }
    }

    return 0;
}

int premultiplyColor(void *, void *, void *, int32_t, void *) {
    qInfo() << "premultiplyColor";
    abort();
    return 0;
}

int premultiplyColor16(void *, void *, void *, int32_t, void *) {
    qInfo() << "premultiplyColor16";
    abort();
    return 0;
}

int premultiplyColorFloat(void *, void *, void *, int32_t, void *) {
    qInfo() << "premultiplyColorFloat";
    abort();
    return 0;
}

double fixedToDouble(int32_t fixed) { return (double)(fixed) / 65536.0; }
int32_t doubleToIFixed(double value) { return value * 65536.0; }

// TODO: check if it is actually correct (does it want 0-255 or 0-1?)
int32_t hlsToRgb(void *, int32_t hls[3], ARGB *rgb) {
    // qInfo() << hls[0].number << hls[1].number << hls[2].number;
    // qInfo() << fixedToDouble(hls[0]) << fixedToDouble(hls[1])
    //         << fixedToDouble(hls[2]);
    QColor color =
        QColor::fromHslF(fmod(fixedToDouble(hls[0]) + 360, 360) / 360.,
                         fixedToDouble(hls[2]), fixedToDouble(hls[1]));
    rgb->r = color.red();
    rgb->g = color.green();
    rgb->b = color.blue();
    return 0;
}

int32_t rgbToHls(void *, ARGB *rgb, int32_t hls[3]) {
    QColor color = QColor::fromRgb(rgb->r, rgb->g, rgb->b);
    float hue = color.hslHueF();
    if (hue == -1) {
        hue = 0;
    }
    hls[0] = doubleToIFixed(hue * 360);
    hls[1] = doubleToIFixed(color.lightnessF());
    hls[2] = doubleToIFixed(color.saturationF());
    return 0;
}
int32_t rgbToYiq(void *, ARGB *rgb, int32_t yiq[3]) {
    yiq[0] =
        doubleToIFixed(rgb->r * 0.299 + rgb->g * 0.587 + rgb->b * 0.114) / 255.;
    yiq[1] =
        doubleToIFixed(rgb->r * 0.596 - rgb->g * 0.275 - rgb->b * 0.321) / 255.;
    yiq[2] =
        doubleToIFixed(rgb->r * 0.212 - rgb->g * 0.523 + rgb->b * 0.311) / 255.;
    return 0;
}
int32_t yiqToRgb(void *, int32_t yiq[3], ARGB *rgb) {
    double y = fixedToDouble(yiq[0]);
    double i = fixedToDouble(yiq[1]);
    double q = fixedToDouble(yiq[2]);
    rgb->r = saturate(y + 0.956 * i + 0.619 * q) * 255;
    rgb->g = saturate(y - 0.272 * i - 0.647 * q) * 255;
    rgb->b = saturate(y - 1.106 * i + 1.703 * q) * 255;
    return 0;
}
int32_t luminance(void *, ARGB *rgb, int32_t *value) {
    *value =
        doubleToIFixed((rgb->r * 0.2 + rgb->g * 0.7 + rgb->b * 0.1) / 255.);
    return 0;
}
int32_t hue(void *, ARGB *rgb, int32_t[3], int32_t *value) {
    QColor color = QColor::fromRgb(rgb->r, rgb->g, rgb->b);
    *value = doubleToIFixed(color.hueF());
    return 0;
}
int32_t lightness(void *, ARGB *rgb, int32_t *value) {
    QColor color = QColor::fromRgb(rgb->r, rgb->g, rgb->b);
    *value = doubleToIFixed(color.lightnessF());
    return 0;
}
int32_t saturation(void *, ARGB *rgb, int32_t *value) {
    QColor color = QColor::fromRgb(rgb->r, rgb->g, rgb->b);
    *value = doubleToIFixed(color.saturationF());
    return 0;
}

int32_t subpixelSample(void *, Fixed x, Fixed y, SubpixelSampleData *data,
                       ARGB *dest) {
    // qInfo() << "subpixelSample";
    int finalX = x.number;
    int finalY = y.number;
    if (finalX < 0 || finalY < 0 || finalX >= (int)data->source->width ||
        finalY >= (int)data->source->height) {
        return 0;
    }
    *dest = data->source->data[finalY * data->source->width + finalX];
    return 0;
}

int32_t blend(void *, AELayer *a, AELayer *b, Fixed ratio, AELayer *dest) {
    qInfo() << "blend";
    double ratio2 = saturate(fixedToDouble(ratio));
    qInfo() << "ratio" << ratio2;
    qInfo() << "a" << a;
    qInfo() << "b" << b;
    qInfo() << "dest" << dest;
    if (a->width != b->width || a->height != b->height) {
        qCritical() << "Cannot blend: different sizes";
        return 1;
    }

    size_t byteCount = a->width * a->height * 4;

    if (ratio2 == 0) {
        if (dest->data != a->data) {
            memcpy(dest->data, a->data, byteCount);
        }
        return 0;
    } else if (ratio2 == 1) {
        if (dest->data != b->data) {
            memcpy(dest->data, b->data, byteCount);
        }
        return 0;
    }

    double negRatio = 1 - ratio2;

    ARGB *destPixels = new ARGB[byteCount];
    for (uint32_t i = 0; i < a->width * a->height; i++) {
        destPixels[i].a = a->data[i].a * negRatio + b->data[i].a * ratio2;
        destPixels[i].r = a->data[i].r * negRatio + b->data[i].r * ratio2;
        destPixels[i].g = a->data[i].g * negRatio + b->data[i].g * ratio2;
        destPixels[i].b = a->data[i].b * negRatio + b->data[i].b * ratio2;
    }
    memcpy(dest->data, destPixels, byteCount);
    delete[] destPixels;

    return 0;
}

int32_t copy(void *effect, AELayer *src, AELayer *dst, AERect *srcRect,
             AERect *dstRect) {
    qInfo() << "copy() CALLED";
    qInfo() << "    effect" << effect;
    qInfo() << "    src" << src;
    qInfo() << "    dst" << dst;
    qInfo() << "    srcRect" << srcRect;
    qInfo() << "    dstRect" << dstRect;
    AERect srcRect2 = src->rect;
    AERect dstRect2 = dst->rect;
    if (srcRect) {
        srcRect2 = *srcRect;
    }
    if (dstRect) {
        dstRect2 = *dstRect;
    }
    int srcX = srcRect2.left;
    int srcY = srcRect2.top;
    int srcW = srcRect2.right - srcRect2.left;
    int srcH = srcRect2.bottom - srcRect2.top;

    int dstX = dstRect2.left;
    int dstY = dstRect2.top;
    int dstW = dstRect2.right - dstRect2.left;
    int dstH = dstRect2.bottom - dstRect2.top;

    qInfo() << "    srcRect  (x,y,w,h)" << srcX << srcY << srcW << srcH;
    qInfo() << "    dstRect  (x,y,w,h)" << dstX << dstY << dstW << dstH;
    qInfo() << "    srcLayer (l,t,r,b)" << src->rect.left << src->rect.top
            << src->rect.right << src->rect.bottom;
    qInfo() << "    dstLayer (l,t,r,b)" << dst->rect.left << dst->rect.top
            << dst->rect.right << dst->rect.bottom;

    if (dstX < 0 || dstY < 0 || dstRect2.right > dst->width ||
        dstRect2.bottom > dst->height) {
        qInfo() << "    weird dst rect?";
        return 1;
    }

    // TODO: more checking with different widths

    SwsContext *swsCtx =
        sws_getContext(srcW, srcH, AV_PIX_FMT_ARGB, dstW, dstH, AV_PIX_FMT_ARGB,
                       SWS_BILINEAR, nullptr, nullptr, nullptr);
    int srcStride[] = {src->stride};
    int dstStride[] = {dst->stride};
    ARGB *srcData = src->data + srcY * src->width + srcX;
    ARGB *dstData = dst->data + dstY * dst->width + dstX;
    sws_scale(swsCtx, (const uint8_t *const *)&srcData, srcStride, 0, srcH,
              (uint8_t *const *)&dstData, dstStride);
    sws_freeContext(swsCtx);

    qInfo() << "    copied";

    return 0;
}

int32_t iterateOrigin(InData *in_data, int32_t, int32_t, AELayer *src,
                      AERect *area, Vector2DInt *origin, void *data,
                      iterateFunc_t pixelFunction, AELayer *dst) {
    qInfo() << "iterateOrigin() CALLED";
    qInfo() << "src" << src;
    qInfo() << "dst" << dst;
    int areaX = 0;
    int areaY = 0;
    int areaW = dst->width;
    int areaH = dst->height;
    if (area != nullptr) {
        qInfo() << "has area";
        qInfo() << "area.top" << area->top;
        qInfo() << "area.left" << area->left;
        qInfo() << "area.right" << area->right;
        qInfo() << "area.bottom" << area->bottom;
        areaX = area->left;
        areaY = area->top;
        areaW = area->right - area->left;
        areaH = area->bottom - area->top;
    }
    if (origin != nullptr) {
        qInfo() << "has origin";
        qInfo() << "origin.x" << origin->x;
        qInfo() << "origin.y" << origin->y;
    }
    qInfo() << "Run pixel func";
    qInfo() << "function:" << ((void *)pixelFunction);
    int index = 0;
    for (int y = 0; y < areaH; y++) {
        for (int x = 0; x < areaW; x++) {
            pixelFunction(
                data, x, y,
                src == nullptr
                    ? nullptr
                    : src->data + pixelIndex(x + areaX, y + areaY, src->width),
                dst->data + pixelIndex(x + areaX, y + areaY, dst->width));
        }
    }
    qInfo() << "Finished pixel function";
    return 0;
}

int32_t iterate(InData *in_data, int32_t start, int32_t end, AELayer *src,
                AERect *area, void *data, iterateFunc_t pixelFunction,
                AELayer *dst) {
    qInfo() << "iterate() CALLED";
    iterateOrigin(in_data, start, end, src, area, nullptr, data, pixelFunction,
                  dst);
    return 0;
}

int32_t premultiply(void *, int32_t, AELayer *) {
    qInfo() << "premultiply() CALLED";
    qInfo() << "NOT IMPLEMENTED YET";
    return 0;
}

int32_t iterateGeneric(int32_t, void *, void *) {
    qInfo() << "iterateGeneric";
    abort();
    return 0;
}

int32_t iterate16(InData *, int32_t, int32_t, AELayer *, AERect *, void *,
                  void *, AELayer *) {
    qInfo() << "iterate16";
    abort();
    return 0;
}
int32_t iterateOrigin16(InData *, int32_t, int32_t, AELayer *, AERect *,
                        Vector2DInt *, void *, void *, AELayer *) {
    qInfo() << "iterateOrigin16";
    abort();
    return 0;
}

int32_t transferRect(void *effect, int32_t quality, int32_t flags,
                     int32_t field, AERect *srcRect, AELayer *src,
                     AECompositeMode *compositeMode, void *mask, int32_t destX,
                     int32_t destY, AELayer *dest) {
    qInfo() << "transferRect";
    qInfo() << "    src width height" << src->width << src->height;
    qInfo() << "    dst width height" << dest->width << dest->height;
    qInfo() << "    destX" << destX;
    qInfo() << "    destY" << destY;
    if (srcRect) {
        qInfo() << "    has src rect";
        qInfo() << "        src.left" << srcRect->left;
        qInfo() << "        src.top" << srcRect->top;
        qInfo() << "        src.right" << srcRect->right;
        qInfo() << "        src.bottom" << srcRect->bottom;
    }
    for (int y = srcRect->top; y < srcRect->bottom; y++) {
        for (int x = srcRect->left; x < srcRect->right; x++) {
            dest->data[(y + destY) * dest->width + (x + destX)] =
                src->data[y * src->width + x];
        }
    }
    return 0;
}

AEMemoryHandle *createMemory(size_t size) {
    AEMemoryHandle *handle = new AEMemoryHandle();
    handle->data = malloc(size);
    memset(handle->data, 0, size);
    handle->size = size;
    return handle;
}

void *useMemory(AEMemoryHandle *handle) {
    // qInfo() << "useMemory";
    return handle->data;
}

void freeMemoryUsage(AEMemoryHandle *handle) {
    // qInfo() << "freeMemoryUsage";
}

void freeMemory(AEMemoryHandle *handle) {
    // qInfo() << "freeMemory";
    if (handle == nullptr)
        return;
    delete handle;
}

int32_t createWorld(void *, int32_t width, int32_t height, int32_t,
                    AELayer *layer) {
    qInfo() << "createWorld";
    qInfo() << "layer" << layer;
    layer->width = width;
    layer->height = height;
    layer->rect = {0, 0, width, height};
    layer->stride = width * 4;
    layer->pixelAspectRatio = {1, 1};
    layer->originX = 0;
    layer->originY = 0;
    layer->data = new ARGB[width * height];
    memset(layer->data, 0, width * height * 4);
    return 0;
}

int32_t deleteWorld(void *, AELayer *layer) {
    qInfo() << "deleteWorld";
    delete[] layer->data;
    return 0;
}

int32_t FLT_FastBlur(InData *a, OutData *b, double c, int32_t *d) {
    qInfo() << "FLT_FastBlur" << a << b << c << *d;
    qInfo() << a->width << a->height;
    qInfo() << b->width << b->height;
    return 0;
}

int32_t getCallbackAddress(void *, int32_t, int32_t, int32_t function,
                           void **functionPointer) {
    qInfo() << "getCallbackAddress";
    qInfo() << "function" << function;
    std::vector<void *> functions = {
        nullptr,                    // PF_Callback_NONE,
        nullptr,                    // PF_Callback_BEGIN_SAMPLING,
        (void *)subpixelSample,     // PF_Callback_SUBPIXEL_SAMPLE,
        nullptr,                    // PF_Callback_AREA_SAMPLE,
        nullptr,                    // PF_Callback_OBSOLETE0,
        nullptr,                    // PF_Callback_END_SAMPLING,
        nullptr,                    // PF_Callback_COMPOSITE_RECT,
        (void *)blend,              // PF_Callback_BLEND,
        nullptr,                    // PF_Callback_CONVOLVE,
        (void *)copy,               // PF_Callback_COPY,
        (void *)fill,               // PF_Callback_FILL,
        nullptr,                    // PF_Callback_GAUSSIAN,
        (void *)iterate,            // PF_Callback_ITERATE,
        (void *)premultiply,        // PF_Callback_PREMUL,
        (void *)premultiplyColor,   // PF_Callback_PREMUL_COLOR,
        (void *)rgbToHls,           // PF_Callback_RGB_TO_HLS,
        (void *)hlsToRgb,           // PF_Callback_HLS_TO_RGB,
        (void *)rgbToYiq,           // PF_Callback_RGB_TO_YIQ,
        (void *)yiqToRgb,           // PF_Callback_YIQ_TO_RGB,
        (void *)luminance,          // PF_Callback_LUMINANCE,
        (void *)hue,                // PF_Callback_HUE,
        (void *)lightness,          // PF_Callback_LIGHTNESS,
        (void *)saturation,         // PF_Callback_SATURATION,
        (void *)createWorld,        // PF_Callback_NEW_WORLD,
        (void *)deleteWorld,        // PF_Callback_DISPOSE_WORLD,
        (void *)iterateOrigin,      // PF_Callback_ITERATE_ORIGIN,
        nullptr,                    // PF_Callback_ITERATE_LUT,
        (void *)transferRect,       // PF_Callback_TRANSFER_RECT,
        nullptr,                    // PF_Callback_TRANSFORM_WORLD,
        nullptr,                    // PF_Callback_ITERATE_ORIGIN_NON_CLIP_SRC,
        nullptr,                    // PF_Callback_ITERATE_GENERIC,
        nullptr,                    // PF_Callback_SUBPIXEL_SAMPLE16,
        nullptr,                    // PF_Callback_AREA_SAMPLE16,
        nullptr,                    // PF_Callback_FILL16,
        (void *)premultiplyColor16, // PF_Callback_PREMUL_COLOR16,
        (void *)iterate16,          // PF_Callback_ITERATE16,
        (void *)iterateOrigin16,    // PF_Callback_ITERATE_ORIGIN16,
        nullptr, // PF_Callback_ITERATE_ORIGIN_NON_CLIP_SRC16,
        nullptr, // PF_Callback_ITERATE_GENERIC_NO_MAX_THREADS,
        nullptr, // PF_Callback_ITERATE_NO_MAX_THREADS,
        nullptr, // PF_Callback_ITERATE_ORIGIN_NO_MAX_THREADS,
        nullptr, // PF_Callback_ITERATE_ORIGIN_NON_CLIP_SRC_NO_MAX_THREADS,
        nullptr, // PF_Callback_ITERATE16_NO_MAX_THREADS,
        nullptr, // PF_Callback_ITERATE_ORIGIN16_NO_MAX_THREADS,
        nullptr, // PF_Callback_ITERATE_ORIGIN_NON_CLIP_SRC16_NO_MAX_THREADS,
    };

    if (function == -2) {
        *functionPointer = (void *)FLT_FastBlur;
        return 0;
    }

    // TODO: negative functions
    void *theFunction = functions[abs(function)];
    if (theFunction == nullptr) {
        qInfo() << "function" << function << "not implemented";
        abort();
    }
    *functionPointer = theFunction;
    return 0;
}

UtilityCallbacks::UtilityCallbacks() {
    subpixelSample = ::subpixelSample;
    blend = ::blend;
    copy = ::copy;
    fill = ::fill;
    iterate = ::iterate;
    premultiply = ::premultiply;
    createWorld = ::createWorld;
    deleteWorld = ::deleteWorld;
    iterateOrigin = ::iterateOrigin;
    transferRect = ::transferRect;
    createMemory = ::createMemory;
    useMemory = ::useMemory;
    freeMemoryUsage = ::freeMemoryUsage;
    freeMemory = ::freeMemory;
    getCallbackAddress = ::getCallbackAddress;

    atan = std::atan;
    atan2 = std::atan2;
    ceil = std::ceil;
    cos = std::cos;
    exp = std::exp;
    fabs = std::fabs;
    floor = std::floor;
    fmod = std::fmod;
    log = std::atan;
    log10 = std::log10;
    pow = std::pow;
    sin = std::sin;
    sqrt = std::sqrt;
    tan = std::tan;
    asin = std::asin;
    acos = std::acos;

    sprintf = std::sprintf;
    strcpy = std::strcpy;

    rgbToHls = ::rgbToHls;
    hlsToRgb = ::hlsToRgb;
    rgbToYiq = ::rgbToYiq;
    yiqToRgb = ::yiqToRgb;
    luminance = ::luminance;
    hue = ::hue;
    lightness = ::lightness;
    saturation = ::saturation;

    iterateGeneric = ::iterateGeneric;
    iterate16 = ::iterate16;
    iterateOrigin16 = ::iterateOrigin16;
}
