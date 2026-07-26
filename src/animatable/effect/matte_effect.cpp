#include "matte_effect.hpp"
#include "math.hpp"
#include "render.hpp"

MatteEffect::MatteEffect() {}

bool MatteEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {

    auto snippet = renderThread->getSnippet(selection);
    if (!snippet.values)
        return false;

    Rect rect = renderBox;
    int offsetX = rect.x - snippet.rect.x;
    int offsetY = rect.y - snippet.rect.y;
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            int sx = x + offsetX;
            int sy = y + offsetY;
            if (sx < 0 || sy < 0 || sx >= snippet.rect.w ||
                sy >= snippet.rect.h)
                continue;

            float value =
                (snippet.values[pixelIndex(sx, sy, snippet.rect.w)] >> 24) /
                255.;

            auto [r, g, b, a] = extractRGBA(source[pixelIndex(x, y, rect.w)]);

            target[pixelIndex(x, y, rect.w)] = makePixel(r, g, b, a * value);
        }
    }
    return true;
}
