#include "invert_effect.hpp"

AnimatableRender *InvertEffect::createClass() {
    return new InvertEffectRender();
}

bool InvertEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                                uint32_t *target) {
    for (int i = 0; i < sourceRect.w * sourceRect.h; i++) {
        target[i] = source[i] ^ 0x00FFFFFF;
    }
    return true;
}
