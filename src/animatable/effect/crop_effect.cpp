#include "crop_effect.hpp"
#include "math.hpp"

CropEffect::CropEffect() {
    left.setMin(0);
    right.setMin(0);
    top.setMin(0);
    bottom.setMin(0);
}

bool CropEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Rect rect = renderBox;
    int finalX = left;
    int finalY = top;
    int finalW = sourceRect.w - right;
    int finalH = sourceRect.h - bottom;
    for (int y = finalY; y < finalH; y++) {
        for (int x = finalX; x < finalW; x++) {
            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(x, y, sourceRect.w)];
        }
    }
    return true;
}
