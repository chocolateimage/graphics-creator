#include "brightness_effect.hpp"
#include "math.hpp"

BrightnessEffect::BrightnessEffect() {
    brightness.setMin(0);
    brightness.setMax(25500);
    brightness.suffix = "%";
}

bool BrightnessEffectRender::render(const uint32_t *source,
                                    const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderBox;
    double brightness = this->brightness / 100.;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);
            r = std::min(r * brightness, 255.);
            g = std::min(g * brightness, 255.);
            b = std::min(b * brightness, 255.);
            target[pixelIndex(x, y, rect.w)] = makePixel(r, g, b, a);
        }
    }
    return true;
}
