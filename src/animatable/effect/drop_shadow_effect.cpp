#include "drop_shadow_effect.hpp"
#include "math.hpp"

DropShadowEffect::DropShadowEffect() {}

Rect DropShadowEffectRender::getRenderBox(const Rect &lastBox) {
    auto [xDist, yDist] = distS();
    return EffectRender::getRenderBox(lastBox).united(
        {lastBox.x + xDist, lastBox.y + yDist, lastBox.w + xDist,
         lastBox.h + yDist});
}

std::pair<int, int> DropShadowEffectRender::distS() {
    return {distance * std::cos(angle * (M_PI / 180.) - M_PI_2),
            distance * std::sin(angle * (M_PI / 180.) - M_PI_2)};
}

bool DropShadowEffectRender::render(const uint32_t *source,
                                    const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderBox;

    auto [xDist, yDist] = distS();
    Color c = color;
    double ca = c.a / 255.;

    int shadowX = 0;
    int shadowY = 0;
    int textX = 0;
    int textY = 0;

    if (xDist > 0) {
        shadowX = xDist;
    } else {
        textX = xDist;
    }
    if (yDist > 0) {
        shadowY = yDist;
    } else {
        textY = yDist;
    }

    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            uint8_t alpha = source[pixelIndex(x, y, sourceRect.w)] >> 24;
            target[pixelIndex(x + shadowX, y + shadowY, rect.w)] =
                makePixel(c.r, c.g, c.b, alpha * ca);
        }
    }

    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            int targetIndex = pixelIndex(x - textX, y - textY, rect.w);
            target[targetIndex] = over(target[targetIndex],
                                       source[pixelIndex(x, y, sourceRect.w)]);
        }
    }

    return true;
}
