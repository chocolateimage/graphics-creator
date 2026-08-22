#include "displacement_map_effect.hpp"
#include "math.hpp"
#include "render.hpp"

DisplacementMapEffect::DisplacementMapEffect() {}

bool DisplacementMapEffectRender::render(const uint32_t *source,
                                         const Rect &sourceRect,
                                         uint32_t *target) {
    auto snippet = renderThread->getSnippet(selection);
    if (!snippet.values)
        return false;

    Rect rect = renderBox;

    int ox = rect.x - snippet.rect.x;
    int oy = rect.y - snippet.rect.y;
    int disX = xDisplacement;
    int disY = yDisplacement;

    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            int sx = x;
            int sy = y;

            int snipX = x + ox;
            int snipY = y + oy;
            if (snipX >= 0 && snipY >= 0 && snipX < snippet.rect.w &&
                snipY < snippet.rect.h) {
                auto [r, g, b, a] = extractRGBA(
                    snippet.values[pixelIndex(snipX, snipY, snippet.rect.w)]);
                double origVal = (r * 0.2 + g * 0.7 + b * 0.1) / 255.;
                double val = (origVal - 0.5) * 2 * (a / 255.);
                sx -= val * disX;
                sy -= val * disY;
            }

            if (sx < 0 || sy < 0 || sx >= sourceRect.w || sy >= sourceRect.h)
                continue;

            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(sx, sy, sourceRect.w)];
        }
    }
    return true;
}
