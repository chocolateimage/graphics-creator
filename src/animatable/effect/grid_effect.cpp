#include "grid_effect.hpp"
#include "animatable/element/element.hpp"
#include "math.hpp"

GridEffect::GridEffect() {
    boxWidth.setMin(1);
    boxHeight.setMin(1);
    lineWidth.setMin(0);
    lineHeight.setMin(0);
}

bool GridEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    int ox = sourceRect.x - element->x;
    int oy = sourceRect.y - element->y;
    uint32_t bg;
    uint32_t fg;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            int index = pixelIndex(x, y, sourceRect.w);
            int sx = std::abs(x + ox);
            int sy = std::abs(y + oy);
            int mx = sx % boxWidth;
            int my = sy % boxHeight;
            if (mx < lineWidth || my < lineHeight) {
                target[index] =
                    makePixel(lineColor.get().r, lineColor.get().g,
                              lineColor.get().b, source[index] >> 24);
            } else {
                target[index] =
                    makePixel(backgroundColor.get().r, backgroundColor.get().g,
                              backgroundColor.get().b, source[index] >> 24);
            }
        }
    }
    return true;
}
