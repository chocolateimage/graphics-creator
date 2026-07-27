#include "drop_shadow_effect.hpp"
#include "math.hpp"

DropShadowEffect::DropShadowEffect() {
    angle.suffix = "°";
    distance.setMin(0);
    softness.setMin(0);
}

Rect DropShadowEffectRender::getRenderBox(const Rect &lastBox) {
    int dc = std::ceil(distance) + softness * 2;
    return {lastBox.x - dc, lastBox.y - dc, lastBox.w + dc * 2,
            lastBox.h + dc * 2};
}

static int getAlpha(int x, int y, const uint32_t *values, const Rect &rect) {
    if (x < 0 || y < 0)
        return 0;
    if (x >= rect.w || y >= rect.h)
        return 0;
    return values[pixelIndex(x, y, rect.w)] >> 24;
}

bool DropShadowEffectRender::render(const uint32_t *source,
                                    const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderBox;

    int dc = std::ceil(distance) + softness * 2;
    int xDist = distance * std::cos(angle * (M_PI / 180.) - M_PI_2);
    int yDist = distance * std::sin(angle * (M_PI / 180.) - M_PI_2);
    Color c = color;
    double ca = c.a / 255.;

    int shadowX = dc + xDist;
    int shadowY = dc + yDist;
    int textX = dc;
    int textY = dc;

    int radius = softness;
    int windowSize = radius * 2 + 1;

    for (int y = 0; y < rect.h; y++) {
        int wa = 0;
        for (int x = 0; x < rect.w; x++) {
            int alphaLeft = (getAlpha(x - radius - 1 - shadowX, y - shadowY,
                                      source, sourceRect));
            int alphaRight = (getAlpha(x + radius - shadowX, y - shadowY,
                                       source, sourceRect));
            wa += alphaRight - alphaLeft;
            target[pixelIndex(x, y, rect.w)] =
                makePixel(c.r, c.g, c.b, ((float)wa / windowSize) * ca);
        }
    }

    for (int x = 0; x < rect.w; x++) {
        int wa = 0;
        for (int y = 0; y < rect.h; y++) {
            int alphaLeft = (getAlpha(x, y - radius - 1, target, rect));
            int alphaRight = (getAlpha(x, y + radius, target, rect));
            wa += alphaRight - alphaLeft;
            target[pixelIndex(x, y, rect.w)] =
                makePixel(c.r, c.g, c.b,
                          std::clamp((int)((float)wa / windowSize), 0, 255));
        }
    }

    if (!shadowOnly) {
        for (int y = 0; y < sourceRect.h; y++) {
            for (int x = 0; x < sourceRect.w; x++) {
                int targetIndex = pixelIndex(x + textX, y + textY, rect.w);
                target[targetIndex] =
                    over(target[targetIndex],
                         source[pixelIndex(x, y, sourceRect.w)]);
            }
        }
    }

    return true;
}
