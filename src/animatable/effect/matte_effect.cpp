#include "matte_effect.hpp"
#include "math.hpp"
#include "render.hpp"

MatteEffect::MatteEffect() {
    useForMatte.enumList.push_back("Alpha");
    useForMatte.enumList.push_back("Luminance");
    useForMatte.updateBoundsToEnumList();
}

bool MatteEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {

    auto snippet = renderThread->getSnippet(selection);
    if (!snippet.values)
        return false;

    Rect rect = renderBox;
    int offsetX = rect.x - snippet.rect.x;
    int offsetY = rect.y - snippet.rect.y;
    bool invert = this->invert;
    int useForMatte = this->useForMatte;
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            int sx = x + offsetX;
            int sy = y + offsetY;
            float value = 0;
            if (sx >= 0 && sy >= 0 && sx < snippet.rect.w &&
                sy < snippet.rect.h) {
                uint32_t snippetValue =
                    snippet.values[pixelIndex(sx, sy, snippet.rect.w)];
                if (useForMatte == 0) {
                    value = (snippetValue >> 24) / 255.;
                } else if (useForMatte == 1) {
                    auto [r, g, b, a] = extractRGBA(snippetValue);
                    value = (r * 0.2 + g * 0.7 + b * 0.1) / 255.;
                }
            }

            if (invert) {
                value = 1 - value;
            }

            if (value == 0)
                continue;

            auto [r, g, b, a] = extractRGBA(source[pixelIndex(x, y, rect.w)]);

            target[pixelIndex(x, y, rect.w)] = makePixel(r, g, b, a * value);
        }
    }
    return true;
}
