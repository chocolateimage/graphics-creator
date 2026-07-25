#include "place_original_effect.hpp"
#include "math.hpp"

PlaceOriginalEffect::PlaceOriginalEffect() {}

Rect PlaceOriginalEffectRender::getRenderBox(const Rect &lastBox) {
    return lastBox.united(originalBox);
}

bool PlaceOriginalEffectRender::render(const uint32_t *source,
                                       const Rect &sourceRect,
                                       uint32_t *target) {
    Rect newRect = renderBox;
    Rect rect = originalBox;

    int sox = sourceRect.x - newRect.x;
    int soy = sourceRect.y - newRect.y;
    int ox = rect.x - newRect.x;
    int oy = rect.y - newRect.y;

    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            int tx = sox + x;
            int ty = soy + y;
            if (tx < 0 || tx >= newRect.w || ty < 0 || ty >= newRect.h)
                continue;

            target[pixelIndex(tx, ty, newRect.w)] =
                source[pixelIndex(x, y, sourceRect.w)];
        }
    }

    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            int tx = ox + x;
            int ty = oy + y;
            if (tx < 0 || tx >= newRect.w || ty < 0 || ty >= newRect.h)
                continue;

            target[pixelIndex(tx, ty, newRect.w)] =
                over(target[pixelIndex(tx, ty, newRect.w)],
                     originalValues[pixelIndex(x, y, rect.w)]);
        }
    }
    return true;
}
