#include "in_out_effect.hpp"

InOutEffect::InOutEffect() {}

bool InOutEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {
    bool isOutside = currentSeconds < in || currentSeconds > out;
    if (invert) {
        isOutside = !isOutside;
    }
    if (isOutside)
        return true;

    memcpy(target, source, sourceRect.w * sourceRect.h * 4);
    return true;
}
