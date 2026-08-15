#include "hsv_effect.hpp"
#include "hsv.hpp"
#include "math.hpp"

HsvEffect::HsvEffect() {
    hue.suffix = "°";
    saturation.setMin(-100);
    saturation.setMax(100);
    lightness.setMin(-100);
    lightness.setMax(100);
}

bool HsvEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                             uint32_t *target) {
    Rect rect = renderBox;
    float hue = this->hue.get();
    float sat = this->saturation.get() / 100.;
    float lig = this->lightness.get() / 100.;
    uint8_t targetLig = lig < 0 ? 0 : 255;
    lig = std::abs(lig);
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);
            HSV hsv = rgb2hsv(r / 255., g / 255., b / 255.);
            hsv.h = (hsv.h + hue) / 360.;
            hsv.h = (hsv.h - std::floor(hsv.h)) * 360.;
            hsv.s = std::clamp(hsv.s + sat, 0., 1.);
            // hsv.v = std::clamp(hsv.v + lig, 0., 1.);

            auto [r2, g2, b2] = hsv2rgb(hsv);
            r2 = lerp((uint8_t)(r2 * 255), targetLig, lig);
            g2 = lerp((uint8_t)(g2 * 255), targetLig, lig);
            b2 = lerp((uint8_t)(b2 * 255), targetLig, lig);
            target[pixelIndex(x, y, rect.w)] = makePixel(r2, g2, b2, a);
        }
    }
    return true;
}
