#include "vignette_effect.hpp"
#include "math.hpp"

VignetteEffect::VignetteEffect() {
    from.setMin(0);
    to.setMin(0);

    mode.enumList.push_back("Color");
    mode.enumList.push_back("Alpha");
    mode.updateBoundsToEnumList();
}

bool VignetteEffectRender::render(const uint32_t *source,
                                  const Rect &sourceRect, uint32_t *target) {
    int from = this->from;
    int to = this->to;
    bool useColor = mode == 0;

    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            int index = pixelIndex(x, y, sourceRect.w);

            int dist = distance(x, y, sourceRect.w / 2., sourceRect.h / 2.);

            float mult = smoothstep(from, to, dist);

            if (useColor) {
                mult = 1 - mult;
                auto [r, g, b, a] = extractRGBA(source[index]);
                target[index] = makePixel(r * mult, g * mult, b * mult, a);
            } else {
                target[index] =
                    makePixel(0, 0, 0, (source[index] >> 24) * mult);
            }
        }
    }
    return true;
}
