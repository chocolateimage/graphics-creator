#include "crop_effect.hpp"
#include "math.hpp"

CropEffect::CropEffect() {
    left.setMin(0);
    right.setMin(0);
    top.setMin(0);
    bottom.setMin(0);
}

Rect CropEffectRender::getRenderBox(const Rect &lastBox) {
    return {lastBox.x + std::min(left.get(), lastBox.w),
            lastBox.y + std::min(top.get(), lastBox.h),
            std::max(lastBox.w - right - left, 1),
            std::max(lastBox.h - bottom - top, 1)};
}

bool CropEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    if (sourceRect.w - right - left < 1 || sourceRect.h - bottom - top < 1) {
        return true;
    }

    Rect rect = renderBox;
    int finalX = left;
    int finalY = top;
    int finalW = rect.w;
    int finalH = rect.h;
    for (int y = 0; y < finalH; y++) {
        for (int x = 0; x < finalW; x++) {
            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(x + finalX, y + finalY, sourceRect.w)];
        }
    }
    return true;
}
