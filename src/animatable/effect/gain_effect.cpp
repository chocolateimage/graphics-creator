#include "gain_effect.hpp"
#include "math.hpp"

GainEffect::GainEffect() {
    gain.setMin(0);
    gain.setMax(25500);
    gain.suffix = "%";
    gain.displayName = "Gain";
}

bool GainEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Rect rect = renderBox;
    double gain = this->gain / 100.;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);
            r = std::min(r * gain, 255.);
            g = std::min(g * gain, 255.);
            b = std::min(b * gain, 255.);
            target[pixelIndex(x, y, rect.w)] = makePixel(r, g, b, a);
        }
    }
    return true;
}
