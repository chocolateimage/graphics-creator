#include "blur_effect.hpp"
#include "math.hpp"
#include <QDebug>
#include <QElapsedTimer>

AnimatableRender *BlurEffect::createClass() { return new BlurEffectRender(); }

Rect BlurEffectRender::getRenderBox(const Rect &lastBox) {
    return {lastBox.x - radius, lastBox.y - radius, lastBox.w + radius * 2,
            lastBox.h + radius * 2};
}

bool BlurEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Rect rect = renderBox;

    int radius = this->radius;
    int windowSize = radius * 2 + 1;

    uint32_t *tempValues = new uint32_t[rect.w * rect.h];
    memset(tempValues, 0, rect.w * rect.h * 4);

    for (int y = radius; y < rect.h - radius; y++) {
        int r = 0;
        int g = 0;
        int b = 0;
        int a = 0;

        for (int x = 0; x < rect.w; x++) {
            int leftX = x - radius - 1 - radius;
            int rightX = x + radius - radius;
            uint8_t leftR = 0;
            uint8_t leftG = 0;
            uint8_t leftB = 0;
            uint8_t leftA = 0;
            uint8_t rightR = 0;
            uint8_t rightG = 0;
            uint8_t rightB = 0;
            uint8_t rightA = 0;
            if (leftX >= 0 && leftX < sourceRect.w) {
                uint32_t color =
                    source[pixelIndex(leftX, y - radius, sourceRect.w)];
                leftR = color >> 16;
                leftG = color >> 8;
                leftB = color;
                leftA = color >> 24;
            }
            if (rightX >= 0 && rightX < sourceRect.w) {
                uint32_t color =
                    source[pixelIndex(rightX, y - radius, sourceRect.w)];
                rightR = color >> 16;
                rightG = color >> 8;
                rightB = color;
                rightA = color >> 24;
            }

            r += rightR - leftR;
            g += rightG - leftG;
            b += rightB - leftB;
            a += rightA - leftA;

            tempValues[pixelIndex(x, y, rect.w)] = makePixel(
                r / windowSize, g / windowSize, b / windowSize, a / windowSize);
        }
    }

    for (int x = 0; x < rect.w; x++) {
        int r = 0;
        int g = 0;
        int b = 0;
        int a = 0;

        for (int y = 0; y < rect.h; y++) {
            int topY = y - radius - 1;
            int bottomY = y + radius;
            uint8_t leftR = 0;
            uint8_t leftG = 0;
            uint8_t leftB = 0;
            uint8_t leftA = 0;
            uint8_t rightR = 0;
            uint8_t rightG = 0;
            uint8_t rightB = 0;
            uint8_t rightA = 0;
            if (topY >= 0 && topY < rect.h) {
                uint32_t color = tempValues[pixelIndex(x, topY, rect.w)];
                leftR = color >> 16;
                leftG = color >> 8;
                leftB = color;
                leftA = color >> 24;
            }
            if (bottomY >= 0 && bottomY < rect.h) {
                uint32_t color = tempValues[pixelIndex(x, bottomY, rect.w)];
                rightR = color >> 16;
                rightG = color >> 8;
                rightB = color;
                rightA = color >> 24;
            }

            r += rightR - leftR;
            g += rightG - leftG;
            b += rightB - leftB;
            a += rightA - leftA;

            target[pixelIndex(x, y, rect.w)] = makePixel(
                r / windowSize, g / windowSize, b / windowSize, a / windowSize);
        }
    }

    delete[] tempValues;

    return true;
}
