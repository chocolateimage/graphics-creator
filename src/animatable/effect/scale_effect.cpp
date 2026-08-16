#include "scale_effect.hpp"

extern "C" {
#include <libswscale/swscale.h>
}

ScaleEffect::ScaleEffect() {
    scaleX.setMin(0);
    scaleY.setMin(0);
    alignX.setMin(0);
    alignX.setMax(1);
    alignY.setMin(0);
    alignY.setMax(1);

    scaleX.stepMultiplier = 0.05;
    scaleY.stepMultiplier = 0.05;
    alignX.stepMultiplier = 0.05;
    alignY.stepMultiplier = 0.05;

    scaleType.enumList.push_back("Nearest neighbour");
    scaleType.enumList.push_back("Bilinear");
    scaleType.enumList.push_back("Bicubic");
    scaleType.enumList.push_back("Area");
    scaleType.enumList.push_back("Gaussian");
    scaleType.enumList.push_back("Sinc");
    scaleType.enumList.push_back("Lanczos");
    scaleType.enumList.push_back("Spline");
    scaleType.updateBoundsToEnumList();
}

Rect ScaleEffectRender::getRenderBox(const Rect &lastBox) {
    int offsetX = (lastBox.w * alignX) - (lastBox.w * alignX * scaleX);
    int offsetY = (lastBox.h * alignY) - (lastBox.h * alignY * scaleY);
    return {(int)(lastBox.x) + offsetX, (int)(lastBox.y) + offsetY,
            std::max(1, (int)(lastBox.w * scaleX)),
            std::max(1, (int)(lastBox.h * scaleY))};
}

bool ScaleEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {
    Rect rect = renderBox;
    double scaleX = this->scaleX;
    double scaleY = this->scaleY;

    if (scaleX <= 0 || scaleY <= 0) {
        memset(target, 0, rect.w * rect.h * 4);
        return true;
    }

    int scaleFlags = SWS_POINT;
    int scaleType = this->scaleType;
    if (scaleType == 0) {
        scaleFlags = SWS_POINT;
    } else if (scaleType == 1) {
        scaleFlags = SWS_BILINEAR;
    } else if (scaleType == 2) {
        scaleFlags = SWS_BICUBIC;
    } else if (scaleType == 3) {
        scaleFlags = SWS_AREA;
    } else if (scaleType == 4) {
        scaleFlags = SWS_GAUSS;
    } else if (scaleType == 5) {
        scaleFlags = SWS_SINC;
    } else if (scaleType == 6) {
        scaleFlags = SWS_LANCZOS;
    } else if (scaleType == 7) {
        scaleFlags = SWS_SPLINE;
    }

    SwsContext *swsCtx = sws_getContext(
        sourceRect.w, sourceRect.h, AV_PIX_FMT_BGRA, rect.w, rect.h,
        AV_PIX_FMT_BGRA, scaleFlags, nullptr, nullptr, nullptr);
    int strideSource[] = {sourceRect.w * 4};
    int strideTarget[] = {rect.w * 4};
    sws_scale(swsCtx, (const uint8_t *const *)&source, strideSource, 0,
              sourceRect.h, (uint8_t *const *)&target, strideTarget);
    sws_freeContext(swsCtx);

    return true;
}
