#include "twist_effect.hpp"
#include "math.hpp"

TwistEffect::TwistEffect() {
    direction.enumList.push_back("Horizontal");
    direction.enumList.push_back("Vertical");
    direction.setMin(0);
    direction.setMax(1);
}

bool TwistEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {
    Rect rect = renderBox;
    bool isVertical = (bool)this->direction.get();
    bool flip = this->flip;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            float u = (float)x / sourceRect.w;
            float v = (float)y / sourceRect.h;

            float a = std::cos((isVertical ? v : u) * M_PI);
            if (!flip) {
                a = std::abs(a);
            }
            float start = .5 - a / 2.;
            if (isVertical) {
                u = remap(u, start, start + a, 0, 1);
            } else {
                v = remap(v, start, start + a, 0, 1);
            }

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
