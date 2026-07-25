#include "flip_effect.hpp"
#include "math.hpp"

FlipEffect::FlipEffect() {}

bool FlipEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Rect rect = renderBox;
    bool h = horizontal;
    bool v = vertical;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            int sx = h ? (sourceRect.w - x - 1) : x;
            int sy = v ? (sourceRect.h - y - 1) : y;

            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(sx, sy, sourceRect.w)];
        }
    }
    return true;
}
