#include "crt_effect.hpp"
#include "math.hpp"

CrtEffect::CrtEffect() {}

bool CrtEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                             uint32_t *target) {
    Rect rect = renderBox;

    // Based on https://www.shadertoy.com/view/Ms23DR

    float apply = 1.0 + 0.01 * std::sin(110.0 * currentSeconds);

    int w = sourceRect.w;
    int h = sourceRect.h;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float uvX = (float)x / w;
            float uvY = 1 - ((float)y / h);

            uvX = (uvX - 0.5) * 2.0;
            uvY = (uvY - 0.5) * 2.0;
            uvX *= 1.1;
            uvY *= 1.1;
            uvX *= 1.0 + std::pow((std::abs(uvY) / 5.0), 2.0);
            uvY *= 1.0 + std::pow((std::abs(uvX) / 4.0), 2.0);
            uvX = (uvX / 2.0) + 0.5;
            uvY = (uvY / 2.0) + 0.5;
            uvX = uvX * 0.92 + 0.04;
            uvY = uvY * 0.92 + 0.04;

            int sx = uvX * w;
            int sy = (1 - uvY) * h;

            if (sx < 0 || sx >= w || sy < 0 || sy >= h)
                continue;

            uint32_t num = source[pixelIndex(sx, sy, w)];
            double r, g, b;
            int a;

            r = ((uint8_t)(num >> 16)) / 255.;
            g = ((uint8_t)(num >> 8)) / 255.;
            b = ((uint8_t)num) / 255.;
            a = (uint8_t)(num >> 24);

            r += 0.05;
            g += 0.05;
            b += 0.05;

            r = r * 0.6 + 0.4 * r * r;
            g = g * 0.6 + 0.4 * g * g;
            b = b * 0.6 + 0.4 * b * b;

            float scans =
                0.4 +
                0.7 * std::pow(
                          saturate(0.35 + 0.35 * std::sin(3.5 * currentSeconds +
                                                          uvY * h * 1.5)),
                          1.7);

            float apply2 = 1.0 - 0.65 * saturate(((x % 2) - 1.0) * 2.0);

            float mult = std::pow((0.0 + 1.0 * 16.0 * uvX * uvY * (1.0 - uvX) *
                                             (1.0 - uvY)),
                                  0.3) *
                         scans * apply * apply2;

            r *= 2.8 * 0.95 * mult;
            g *= 2.8 * 1.05 * mult;
            b *= 2.8 * 0.95 * mult;

            target[pixelIndex(x, y, rect.w)] = makePixel(
                saturate(r) * 255, saturate(g) * 255, saturate(b) * 255, a);
        }
    }
    return true;
}
