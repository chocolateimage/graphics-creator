#include "pixelate_effect.hpp"
#include "math.hpp"

PixelateEffect::PixelateEffect() { strength.setMin(1); }

bool PixelateEffectRender::render(const uint32_t *source,
                                  const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderBox;
    int strength = this->strength;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(x / strength * strength,
                                  y / strength * strength, sourceRect.w)];
        }
    }
    return true;
}
