#include "crop_effect.hpp"
#include "math.hpp"

CropEffect::CropEffect() {
    left.setMin(0);
    right.setMin(0);
    top.setMin(0);
    bottom.setMin(0);
}

Rect CropEffectRender::getRenderBox(const Rect &lastBox) {
    return {lastBox.x + left, lastBox.y + top, lastBox.w - right - left,
            lastBox.h - bottom - top};
}

bool CropEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
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
