#include "fill_effect.hpp"
#include "brush.hpp"
#include "math.hpp"

FillEffect::FillEffect() {}

bool FillEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Rect rect = renderBox;
    Brush &fill = this->fill;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            float alpha = (source[pixelIndex(x, y, sourceRect.w)] >> 24) / 255.;
            Color c = getBrushPixel(fill, x, y, sourceRect.w, sourceRect.h);
            target[pixelIndex(x, y, rect.w)] =
                makePixel(c.r, c.g, c.b, c.a * alpha);
        }
    }
    return true;
}
