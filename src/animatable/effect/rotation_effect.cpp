#include "rotation_effect.hpp"
#include "math.hpp"
#include <QRect>

RotationEffect::RotationEffect() {
    angle.suffix = "°";
    pivotX.setMin(0);
    pivotY.setMin(0);
    pivotX.setMax(100);
    pivotY.setMax(100);
    pivotX.suffix = "%";
    pivotY.suffix = "%";
}

Rect RotationEffectRender::getRenderBox(const Rect &lastBox) {
    int xPivot = lastBox.w * pivotX / 100.;
    int yPivot = lastBox.h * pivotY / 100.;
    float angle = this->angle.get() * M_PI / 180.;
    float cosAngle = std::cos(angle);
    float sinAngle = std::sin(angle);

    QRect rect;

    QList<QPoint> points = {QPoint{0, 0}, QPoint{lastBox.w, lastBox.h},
                            QPoint{0, lastBox.h}, QPoint{lastBox.w, 0}};

    for (const auto &point : points) {
        int xShifted = point.x() - xPivot;
        int yShifted = point.y() - yPivot;
        int sx = xPivot + (xShifted * cosAngle - yShifted * sinAngle);
        int sy = yPivot + (xShifted * sinAngle + yShifted * cosAngle);

        if (point.isNull()) {
            rect = QRect{sx, sy, 1, 1};
        } else {
            rect = rect.united(QRect{sx, sy, 1, 1});
        }
    }

    return {rect.x() + lastBox.x, rect.y() + lastBox.y, rect.width(),
            rect.height()};
}

bool RotationEffectRender::render(const uint32_t *source,
                                  const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderBox;
    int xPivot = sourceRect.w * pivotX / 100.;
    int yPivot = sourceRect.h * pivotY / 100.;
    float angle = -this->angle.get() * M_PI / 180.;
    float cosAngle = std::cos(angle);
    float sinAngle = std::sin(angle);
    int offsetX = rect.x - sourceRect.x;
    int offsetY = rect.y - sourceRect.y;

    if (bilinear) {
        for (int y = 0; y < rect.h; y++) {
            for (int x = 0; x < rect.w; x++) {
                int xShifted = x - xPivot + offsetX;
                int yShifted = y - yPivot + offsetY;
                double sx =
                    xPivot + (xShifted * cosAngle - yShifted * sinAngle);
                double sy =
                    yPivot + (xShifted * sinAngle + yShifted * cosAngle);

                int leftX = (int)sx;
                int rightX = leftX + 1;
                int topY = (int)sy;
                int bottomY = topY + 1;
                uint8_t topLeftR = 0, topLeftG = 0, topLeftB = 0, topLeftA = 0;
                uint8_t topRightR = 0, topRightG = 0, topRightB = 0,
                        topRightA = 0;
                uint8_t bottomLeftR = 0, bottomLeftG = 0, bottomLeftB = 0,
                        bottomLeftA = 0;
                uint8_t bottomRightR = 0, bottomRightG = 0, bottomRightB = 0,
                        bottomRightA = 0;

                if (leftX >= 0 && topY >= 0 && leftX < sourceRect.w &&
                    topY < sourceRect.h) {
                    uint32_t color =
                        source[pixelIndex(leftX, topY, sourceRect.w)];
                    topLeftA = color >> 24;
                    topLeftR = ((color >> 16) & 0xff);
                    topLeftG = ((color >> 8) & 0xff);
                    topLeftB = (color & 0xff);
                }
                if (rightX >= 0 && topY >= 0 && rightX < sourceRect.w &&
                    topY < sourceRect.h) {
                    uint32_t color =
                        source[pixelIndex(rightX, topY, sourceRect.w)];
                    topRightA = color >> 24;
                    topRightR = ((color >> 16) & 0xff);
                    topRightG = ((color >> 8) & 0xff);
                    topRightB = (color & 0xff);
                }
                if (leftX >= 0 && bottomY >= 0 && leftX < sourceRect.w &&
                    bottomY < sourceRect.h) {
                    uint32_t color =
                        source[pixelIndex(leftX, bottomY, sourceRect.w)];
                    bottomLeftA = color >> 24;
                    bottomLeftR = ((color >> 16) & 0xff);
                    bottomLeftG = ((color >> 8) & 0xff);
                    bottomLeftB = (color & 0xff);
                }
                if (rightX >= 0 && bottomY >= 0 && rightX < sourceRect.w &&
                    bottomY < sourceRect.h) {
                    uint32_t color =
                        source[pixelIndex(rightX, bottomY, sourceRect.w)];
                    bottomRightA = color >> 24;
                    bottomRightR = ((color >> 16) & 0xff);
                    bottomRightG = ((color >> 8) & 0xff);
                    bottomRightB = (color & 0xff);
                }

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
                int xShifted = x - xPivot + offsetX;
                int yShifted = y - yPivot + offsetY;
                int sx = xPivot + (xShifted * cosAngle - yShifted * sinAngle);
                int sy = yPivot + (xShifted * sinAngle + yShifted * cosAngle);
                if (sx < 0 || sx >= sourceRect.w)
                    continue;
                if (sy < 0 || sy >= sourceRect.h)
                    continue;
                target[pixelIndex(x, y, rect.w)] =
                    source[pixelIndex(sx, sy, sourceRect.w)];
            }
        }
    }
    return true;
}
