#include "place_original_effect.hpp"
#include "math.hpp"

PlaceOriginalEffect::PlaceOriginalEffect() {}

bool PlaceOriginalEffectRender::render(const uint32_t *source,
                                       const Rect &sourceRect,
                                       uint32_t *target) {
    memcpy(target, source, sourceRect.w * sourceRect.h * 4);

    Rect rect = originalBox;
    int ox = rect.x - sourceRect.x;
    int oy = rect.y - sourceRect.y;
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            int tx = ox + x;
            int ty = oy + y;
            if (tx < 0 || ty < 0 || tx >= sourceRect.w || ty >= sourceRect.h)
                continue;

            target[pixelIndex(tx, ty, sourceRect.w)] =
                over(source[pixelIndex(tx, ty, sourceRect.w)],
                     originalValues[pixelIndex(x, y, rect.w)]);
        }
    }
    return true;
}
