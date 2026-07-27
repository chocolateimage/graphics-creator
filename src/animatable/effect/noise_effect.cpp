#include "noise_effect.hpp"
#include "math.hpp"
#include <random>

NoiseEffect::NoiseEffect() {
    mode.enumList.push_back("Alpha");
    mode.enumList.push_back("Color");
    mode.enumList.push_back("Grayscale");
    mode.updateBoundsToEnumList();
}

bool NoiseEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {
    Rect rect = renderBox;
    std::uniform_int_distribution<int> dist(0, 255);
    std::mt19937 random;
    if (perFrame.get()) {
        random.seed(currentFrame);
    } else {
        random.seed(0);
    }
    int mode = this->mode;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            int value = dist(random);
            auto [r, g, b, a] = extractRGBA(source[pixelIndex(x, y, rect.w)]);

            if (mode == 0) {
                target[pixelIndex(x, y, rect.w)] =
                    makePixel(r, g, b, value * (a / 255.));
            } else if (mode == 1) {
                int value2 = dist(random);
                int value3 = dist(random);
                target[pixelIndex(x, y, rect.w)] =
                    makePixel(value * (r / 255.), value2 * (g / 255.),
                              value3 * (b / 255.), a);
            } else if (mode == 2) {
                target[pixelIndex(x, y, rect.w)] =
                    makePixel(value * (r / 255.), value * (g / 255.),
                              value * (b / 255.), a);
            }
        }
    }
    return true;
}
