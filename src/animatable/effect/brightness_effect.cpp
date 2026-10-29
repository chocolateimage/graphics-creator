#include "brightness_effect.hpp"
#include "math.hpp"
#include <immintrin.h>

BrightnessEffect::BrightnessEffect() {
    brightness.setMin(0);
    brightness.setMax(255);
}

__attribute__((target("avx2"))) static int
renderBrightnessAVX(uint8_t brightness, const uint32_t *source,
                    const Rect &sourceRect, uint32_t *target, int size) {
    int i = 0;
    __m256i valueB = _mm256_set_epi8(
        0, brightness, brightness, brightness, 0, brightness, brightness,
        brightness, 0, brightness, brightness, brightness, 0, brightness,
        brightness, brightness, 0, brightness, brightness, brightness, 0,
        brightness, brightness, brightness, 0, brightness, brightness,
        brightness, 0, brightness, brightness, brightness);

    for (; i <= size - 8; i += 8) {
        __m256i valueA = _mm256_loadu_si256((__m256i *)(source + i));
        __m256i result = _mm256_adds_epu8(valueA, valueB);
        _mm256_storeu_si256((__m256i *)(target + i), result);
    }

    return i;
}

bool BrightnessEffectRender::render(const uint32_t *source,
                                    const Rect &sourceRect, uint32_t *target) {
    uint8_t brightness = this->brightness.get();

    int size = sourceRect.w * sourceRect.h;

    int i = 0;

    if (__builtin_cpu_supports("avx2")) {
        i = renderBrightnessAVX(brightness, source, sourceRect, target, size);
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
