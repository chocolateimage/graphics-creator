#include "scale_effect.hpp"
#include "animatable/element/element.hpp"
#include "math.hpp"

ScaleEffect::ScaleEffect() {
    scaleX.setMin(0);
    scaleY.setMin(0);
    alignX.setMin(0);
    alignX.setMax(1);
    alignY.setMin(0);
    alignY.setMax(1);

    scaleX.stepMultiplier = 0.05;
    scaleY.stepMultiplier = 0.05;
    alignX.stepMultiplier = 0.05;
    alignY.stepMultiplier = 0.05;
}

Rect ScaleEffectRender::getRenderBox(const Rect &lastBox) {
    int distX = lastBox.x - element->x;
    int distY = lastBox.y - element->y;
    int offsetX = (element->w * alignX) - (element->w * alignX * scaleX);
    int offsetY = (element->h * alignY) - (element->h * alignY * scaleY);
    return {(int)(element->x + distX * scaleX) + offsetX,
            (int)(element->y + distY * scaleY) + offsetY,
            std::max(1, (int)(lastBox.w * scaleX)),
            std::max(1, (int)(lastBox.h * scaleY))};
}

bool ScaleEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {
    Rect rect = renderBox;
    double scaleX = this->scaleX;
    double scaleY = this->scaleY;

    if (scaleX <= 0 || scaleY <= 0) {
        memset(target, 0, rect.w * rect.h * 4);
        return true;
    }

    if (bilinear) {
        for (int y = 0; y < rect.h; y++) {
            for (int x = 0; x < rect.w; x++) {
                double sx = x / scaleX;
                double sy = y / scaleY;

                int leftX = (int)sx;
                int rightX = std::min(leftX + 1, sourceRect.w - 1);
                int topY = (int)sy;
                int bottomY = std::min(topY + 1, sourceRect.h - 1);
                auto [topLeftR, topLeftG, topLeftB, topLeftA] =
                    extractRGBA(source[pixelIndex(leftX, topY, sourceRect.w)]);
                auto [topRightR, topRightG, topRightB, topRightA] =
                    extractRGBA(source[pixelIndex(rightX, topY, sourceRect.w)]);
                auto [bottomLeftR, bottomLeftG, bottomLeftB, bottomLeftA] =
                    extractRGBA(
                        source[pixelIndex(leftX, bottomY, sourceRect.w)]);
                auto [bottomRightR, bottomRightG, bottomRightB, bottomRightA] =
                    extractRGBA(
                        source[pixelIndex(rightX, bottomY, sourceRect.w)]);

                float xPercent = sx - leftX;
                float yPercent = sy - topY;
                float topValueR = mix(xPercent, topLeftR, topRightR);
                float topValueG = mix(xPercent, topLeftG, topRightG);
                float topValueB = mix(xPercent, topLeftB, topRightB);
                float topValueA = mix(xPercent, topLeftA, topRightA);
                float bottomValueR = mix(xPercent, bottomLeftR, bottomRightR);
                float bottomValueG = mix(xPercent, bottomLeftG, bottomRightG);
                float bottomValueB = mix(xPercent, bottomLeftB, bottomRightB);
                float bottomValueA = mix(xPercent, bottomLeftA, bottomRightA);

                target[pixelIndex(x, y, rect.w)] =
                    makePixel(mix(yPercent, topValueR, bottomValueR),
                              mix(yPercent, topValueG, bottomValueG),
                              mix(yPercent, topValueB, bottomValueB),
                              mix(yPercent, topValueA, bottomValueA));
            }
        }
    } else {
        for (int y = 0; y < rect.h; y++) {
            for (int x = 0; x < rect.w; x++) {
                int sx = x / scaleX;
                int sy = y / scaleY;

                target[pixelIndex(x, y, rect.w)] =
                    source[pixelIndex(sx, sy, sourceRect.w)];
            }
        }
    }
    return true;
}
