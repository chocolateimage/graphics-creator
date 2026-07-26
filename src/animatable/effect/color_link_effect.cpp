#include "color_link_effect.hpp"
#include "math.hpp"
#include "render.hpp"

ColorLinkEffect::ColorLinkEffect() {}

bool ColorLinkEffectRender::render(const uint32_t *source,
                                   const Rect &sourceRect, uint32_t *target) {
    auto snippet = renderThread->getSnippet(selection);
    if (!snippet.values)
        return false;

    uint64_t totalRed{0};
    uint64_t totalGreen{0};
    uint64_t totalBlue{0};
    uint64_t totalAmount{0};

    for (int y = 0; y < snippet.rect.h; y++) {
        for (int x = 0; x < snippet.rect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(snippet.values[pixelIndex(x, y, snippet.rect.w)]);
            totalRed += r;
            totalGreen += g;
            totalBlue += b;
            totalAmount++;
        }
    }

    totalRed /= totalAmount;
    totalGreen /= totalAmount;
    totalBlue /= totalAmount;

    Rect rect = renderBox;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            int index = pixelIndex(x, y, sourceRect.w);
            uint8_t alpha = source[index] >> 24;
            target[index] = makePixel(totalRed, totalGreen, totalBlue, alpha);
        }
    }
    return true;
}
