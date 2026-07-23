#include "opacity_effect.hpp"
#include "math.hpp"

OpacityEffect::OpacityEffect() {
    opacity.setMin(0);
    opacity.setMax(100);
}

bool OpacityEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                                 uint32_t *target) {
    Rect rect = renderBox;
    double opacity = this->opacity / 100.;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            uint32_t value = source[pixelIndex(x, y, sourceRect.w)];
            uint32_t alpha = value >> 24;
            uint32_t rest = value & 0x00FFFFFF;
            uint32_t finalAlpha = (int)(alpha * opacity) << 24;
            target[pixelIndex(x, y, rect.w)] = finalAlpha | rest;
        }
    }
    return true;
}
