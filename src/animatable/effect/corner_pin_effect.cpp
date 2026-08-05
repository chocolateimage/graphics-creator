#include "corner_pin_effect.hpp"
#include "math.hpp"
#include <QRect>

CornerPinEffect::CornerPinEffect() {
    topLeft.stepMultiplier = 0.05;
    topRight.stepMultiplier = 0.05;
    bottomLeft.stepMultiplier = 0.05;
    bottomRight.stepMultiplier = 0.05;
}

// From https://www.shadertoy.com/view/lsBSDm

float cross2d(Vector2DFloat a, Vector2DFloat b) {
    return a.x * b.y - a.y * b.x;
}

Vector2DFloat invBilinear(const Vector2DFloat &p, const Vector2DFloat &a,
                          const Vector2DFloat &b, const Vector2DFloat &c,
                          const Vector2DFloat &d) {
    Vector2DFloat e = b - a;
    Vector2DFloat f = d - a;
    Vector2DFloat g = a - b + c - d;
    Vector2DFloat h = p - a;

    float k2 = cross2d(g, f);
    float k1 = cross2d(e, f) + cross2d(h, g);
    float k0 = cross2d(h, e);

    // if edges are parallel, this is a linear equation
    if (std::abs(k2) < 0.001) {
        return Vector2DFloat((h.x * k1 + f.x * k0) / (e.x * k1 - g.x * k0),
                             -k0 / k1);
    }
    // otherwise, it's a quadratic
    else {
        float w = k1 * k1 - 4.0 * k0 * k2;
        if (w < 0.0)
            return Vector2DFloat(-1.0);
        w = sqrt(w);

        float ik2 = 0.5 / k2;
        float v = (-k1 - w) * ik2;
        float u = (h.x - f.x * v) / (e.x + g.x * v);

        if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0) {
            v = (-k1 + w) * ik2;
            u = (h.x - f.x * v) / (e.x + g.x * v);
        }
        return Vector2DFloat(u, v);
    }
}

Rect CornerPinEffectRender::getRenderBox(const Rect &lastBox) {
    QRect rect = QRect{(int)(lastBox.x + topLeft.get().x * lastBox.w),
                       (int)(lastBox.y + topLeft.get().y * lastBox.h), 1, 1};

    rect = rect.united(QRect{(int)(lastBox.x + topRight.get().x * lastBox.w),
                             (int)(lastBox.y + topRight.get().y * lastBox.h), 1,
                             1});

    rect = rect.united(QRect{(int)(lastBox.x + bottomLeft.get().x * lastBox.w),
                             (int)(lastBox.y + bottomLeft.get().y * lastBox.h),
                             1, 1});

    rect = rect.united(QRect{(int)(lastBox.x + bottomRight.get().x * lastBox.w),
                             (int)(lastBox.y + bottomRight.get().y * lastBox.h),
                             1, 1});

    return Rect{rect.x(), rect.y(), std::min(rect.width(), 5000),
                std::min(rect.height(), 5000)};
}

bool CornerPinEffectRender::render(const uint32_t *source,
                                   const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderBox;
    Vector2DFloat topLeft = this->topLeft;
    Vector2DFloat topRight = this->topRight;
    Vector2DFloat bottomLeft = this->bottomLeft;
    Vector2DFloat bottomRight = this->bottomRight;
    int ox = rect.x - sourceRect.x;
    int oy = rect.y - sourceRect.y;
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            float u = (float)(x + ox) / sourceRect.w;
            float v = (float)(y + oy) / sourceRect.h;
            Vector2DFloat uv =
                invBilinear({u, v}, topLeft, topRight, bottomRight, bottomLeft);
            int sx = uv.x * sourceRect.w;
            int sy = uv.y * sourceRect.h;
            if (sx < 0 || sy < 0 || sx >= sourceRect.w || sy >= sourceRect.h) {
                continue;
            }
            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(sx, sy, sourceRect.w)];
        }
    }
    return true;
}
