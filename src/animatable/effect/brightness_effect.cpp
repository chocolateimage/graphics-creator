#include "brightness_effect.hpp"
#include "math.hpp"
#include <immintrin.h>

BrightnessEffect::BrightnessEffect() {
    brightness.setMin(0);
    brightness.setMax(255);
}

bool BrightnessEffectRender::render(const uint32_t *source,
                                    const Rect &sourceRect, uint32_t *target) {
    uint8_t brightness = this->brightness.get();

    int size = sourceRect.w * sourceRect.h;

    __m256i valueB = _mm256_set_epi8(
        0, brightness, brightness, brightness, 0, brightness, brightness,
        brightness, 0, brightness, brightness, brightness, 0, brightness,
        brightness, brightness, 0, brightness, brightness, brightness, 0,
        brightness, brightness, brightness, 0, brightness, brightness,
        brightness, 0, brightness, brightness, brightness);

    int i = 0;
    for (; i <= size - 8; i += 8) {
        __m256i valueA = _mm256_loadu_si256((__m256i *)(source + i));
        __m256i result = _mm256_adds_epu8(valueA, valueB);
        _mm256_storeu_si256((__m256i *)(target + i), result);
    }

    for (; i < size; i++) {
        RGBA rgba = extractRGBA(source[i]);
        rgba.r = std::min(255, rgba.r + brightness);
        rgba.g = std::min(255, rgba.g + brightness);
        rgba.b = std::min(255, rgba.b + brightness);

        target[i] = makePixel(rgba);
    }

    return true;
}
