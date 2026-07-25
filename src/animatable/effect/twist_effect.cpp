#include "twist_effect.hpp"
#include "math.hpp"

TwistEffect::TwistEffect() {}

bool TwistEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {
    Rect rect = renderBox;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            float u = (float)x / sourceRect.w;
            float v = (float)y / sourceRect.h;

            float a = remap(v, 0, 1, 1, -1);

            float start = .5 - a / 2.;
            u = remap(u, start, start + a, 0, 1);

            int sx = u * sourceRect.w;
            int sy = v * sourceRect.h;

            if (sx < 0 || sy < 0 || sx >= sourceRect.w || sy >= sourceRect.h)
                continue;

            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(sx, sy, sourceRect.w)];
        }
    }
    return true;
}
