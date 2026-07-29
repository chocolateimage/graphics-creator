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
    return true;
}
