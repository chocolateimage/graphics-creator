#include "expand_render_box_effect.hpp"
#include "math.hpp"

ExpandRenderBoxEffect::ExpandRenderBoxEffect() {
    left.setMin(0);
    top.setMin(0);
    right.setMin(0);
    bottom.setMin(0);
    left.setMax(1000);
    top.setMax(1000);
    right.setMax(1000);
    bottom.setMax(1000);
}

Rect ExpandRenderBoxEffectRender::getRenderBox(const Rect &lastBox) {
    return {lastBox.x - left, lastBox.y - top, lastBox.w + right + left,
            lastBox.h + bottom + top};
}

bool ExpandRenderBoxEffectRender::render(const uint32_t *source,
                                         const Rect &sourceRect,
                                         uint32_t *target) {
    Rect rect = renderBox;
    int left = this->left;
    int top = this->top;
    int bottom = this->bottom;
    int right = this->right;
    int ox = left;
    int oy = top;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            target[pixelIndex(x + ox, y + oy, rect.w)] =
                source[pixelIndex(x, y, sourceRect.w)];
        }
    }
    if (showBox) {
        for (int y = 0; y < rect.h; y++) {
            for (int x = 0; x < rect.w; x++) {
                bool isOutside = x < left || y < top || x >= rect.w - right ||
                                 y >= rect.h - bottom;
                if (isOutside) {
                    target[pixelIndex(x, y, rect.w)] = makePixel(
                        255, (x % 2 == 0 && y % 2 == 0) ? 255 : 128, 0, 128);
                } else {
                    target[pixelIndex(x, y, rect.w)] = over(
                        source[pixelIndex(x - left, y - top, sourceRect.w)],
                        makePixel(255, 0, 0, 128));
                }
            }
        }
    }
    return true;
}
