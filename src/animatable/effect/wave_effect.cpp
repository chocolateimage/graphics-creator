#include "wave_effect.hpp"
#include "math.hpp"

WaveEffect::WaveEffect() {
    extend.setMin(0);
    infrequency.setMin(1);
}

Rect WaveEffectRender::getRenderBox(const Rect &lastBox) {
    return {lastBox.x, lastBox.y - extend, lastBox.w, lastBox.h + extend * 2};
}

bool WaveEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Rect rect = renderBox;
    int extend = this->extend;
    int infrequency = this->infrequency;
    int speed = this->speed;
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            int sx = x;
            int sy =
                y +
                sin((double)x / infrequency + currentSeconds * speed) * extend -
                extend;

            if (sy < 0 || sy >= sourceRect.h) {
                continue;
            }

            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(sx, sy, sourceRect.w)];
        }
    }
    return true;
}
