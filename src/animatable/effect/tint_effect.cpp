#include "tint_effect.hpp"
#include "brush.hpp"
#include "math.hpp"

TintEffect::TintEffect() {}

bool TintEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Brush &tint = this->tint;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);
            Color c = getBrushPixel(tint, x, y, sourceRect.w, sourceRect.h);
            float value = (r * 0.2 + g * 0.7 + b * 0.1) / 255.;

            target[pixelIndex(x, y, sourceRect.w)] = makePixel(
                c.r * value, c.g * value, c.b * value, c.a * a / 255.);
        }
    }
    return true;
}
