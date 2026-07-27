#include "tint_effect.hpp"
#include "brush.hpp"
#include "math.hpp"

TintEffect::TintEffect() {
    amountToTint.setMin(0);
    amountToTint.setMax(100);
}

bool TintEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Brush &tint = this->tint;
    float amountToTint = this->amountToTint / 100.;
    Color bc = this->black;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);
            Color c = getBrushPixel(tint, x, y, sourceRect.w, sourceRect.h);
            float value = (r * 0.2 + g * 0.7 + b * 0.1) / 255.;

            target[pixelIndex(x, y, sourceRect.w)] = makePixel(
                lerp(r, (uint8_t)lerp(bc.r, c.r, value), amountToTint),
                lerp(g, (uint8_t)lerp(bc.g, c.g, value), amountToTint),
                lerp(b, (uint8_t)lerp(bc.b, c.b, value), amountToTint),
                lerp(1., c.a / 255., amountToTint) * a);
        }
    }
    return true;
}
