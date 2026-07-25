#include "offset_effect.hpp"

OffsetEffect::OffsetEffect() {}

Rect OffsetEffectRender::getRenderBox(const Rect &lastBox) {
    return {lastBox.x + x, lastBox.y + y, lastBox.w, lastBox.h};
}

bool OffsetEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                                uint32_t *target) {
    Rect rect = renderBox;
    memcpy(target, source, rect.w * rect.h * 4);
    return true;
}
