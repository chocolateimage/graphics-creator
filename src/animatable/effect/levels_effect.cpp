#include "levels_effect.hpp"
#include "math.hpp"

LevelsEffect::LevelsEffect() {
    fromBlack.setMin(0);
    fromWhite.setMin(0);
    toBlack.setMin(0);
    toWhite.setMin(0);
    fromBlack.setMax(255);
    fromWhite.setMax(255);
    toBlack.setMax(255);
    toWhite.setMax(255);
    gamma.setMin(0);
    gamma.setMax(100);
}

bool LevelsEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                                uint32_t *target) {
    Rect rect = renderBox;
    double g2 = 1.f / gamma.get();
    double fromBlack = this->fromBlack;
    double toBlack = this->toBlack;
    double fromWhite = this->fromWhite;
    double toWhite = this->toWhite;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);

            r = mix(std::pow(linearstep(fromBlack, fromWhite, r), g2), toBlack,
                    toWhite);
            g = mix(std::pow(linearstep(fromBlack, fromWhite, g), g2), toBlack,
                    toWhite);
            b = mix(std::pow(linearstep(fromBlack, fromWhite, b), g2), toBlack,
                    toWhite);

            target[pixelIndex(x, y, sourceRect.w)] = makePixel(r, g, b, a);
        }
    }
    return true;
}
