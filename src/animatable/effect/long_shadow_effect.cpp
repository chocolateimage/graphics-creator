#include "long_shadow_effect.hpp"
#include "brush.hpp"
#include "math.hpp"

LongShadowEffect::LongShadowEffect() {
    distance.setMin(0);
    distance.setMax(1000);
}

Rect LongShadowEffectRender::getRenderBox(const Rect &lastBox) {
    return {lastBox.x, lastBox.y, lastBox.w + distance, lastBox.h + distance};
}

bool LongShadowEffectRender::render(const uint32_t *source,
                                    const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderBox;
    int distance = this->distance;
    Brush &fill = this->fill;
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            uint8_t maxAlpha = 0;
            int sx = x;
            int sy = y;
            int dist = distance;
            while (sx > 0 && sy > 0 && dist > 0) {
                if (sx < sourceRect.w && sy < sourceRect.h) {
                    uint8_t a = source[pixelIndex(sx, sy, sourceRect.w)] >> 24;
                    maxAlpha = std::max(a, maxAlpha);
                }
                sx--;
                sy--;
                dist--;
            }
            Color c = getBrushPixel(fill, x, y, rect.w, rect.h);
            target[pixelIndex(x, y, rect.w)] =
                makePixel(c.r, c.g, c.b, c.a * (maxAlpha / 255.));
        }
    }

    if (!shadowOnly) {
        for (int y = 0; y < sourceRect.h; y++) {
            for (int x = 0; x < sourceRect.w; x++) {
                target[pixelIndex(x, y, rect.w)] =
                    over(target[pixelIndex(x, y, rect.w)],
                         source[pixelIndex(x, y, sourceRect.w)]);
            }
        }
    }
    return true;
}
