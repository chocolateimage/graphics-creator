#include "blur_effect.hpp"
#include "math.hpp"
#include <QDebug>
#include <QElapsedTimer>

AnimatableRender *BlurEffect::createClass() { return new BlurEffectRender(); }

bool BlurEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Rect rect = getRenderBox();

    std::array<std::pair<int, int>, 9> offsets = {{
        {-1, -1},
        {+0, -1},
        {+1, -1},
        {-1, +0},
        {+0, +0},
        {+1, +0},
        {-1, +1},
        {+0, +1},
        {+1, +1},
    }};
    int offsetCount = offsets.size();
    QElapsedTimer timer;
    timer.start();
    for (int y = 1; y < sourceRect.h - 1; y++) {
        for (int x = 1; x < sourceRect.w - 1; x++) {
            int r{0};
            int g{0};
            int b{0};
            int a{0};

            for (const auto &[dx, dy] : offsets) {
                const uint32_t color =
                    source[pixelIndex(x + dx, y + dy, sourceRect.w)];
                const uint8_t dr = (uint8_t)(color >> 16);
                const uint8_t dg = (uint8_t)(color >> 8);
                const uint8_t db = (uint8_t)color;
                const uint8_t da = (uint8_t)(color >> 24);

                r += dr;
                g += dg;
                b += db;
                a += da;
            }

            target[pixelIndex(x, y, rect.w)] =
                makePixel(r / offsetCount, g / offsetCount, b / offsetCount,
                          a / offsetCount);
        }
    }
    qInfo() << timer.nsecsElapsed() / 1000. / 1000.;
    return true;
}
