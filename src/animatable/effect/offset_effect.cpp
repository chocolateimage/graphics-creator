#include "offset_effect.hpp"

OffsetEffect::OffsetEffect() {}

Rect OffsetEffectRender::getRenderBox(const Rect &lastBox) {
    if (tiled.get()) {
        return lastBox;
    }
    return {lastBox.x + x, lastBox.y + y, lastBox.w, lastBox.h};
}

bool OffsetEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                                uint32_t *target) {
    if (!tiled.get()) {
        memcpy(target, source, sourceRect.w * sourceRect.h * 4);
        return true;
    }

    int moveX = x.get();
    int moveY = y.get();

    while (moveX < 0) {
        moveX += sourceRect.w;
    }
    while (moveY < 0) {
        moveY += sourceRect.h;
    }

    moveX %= sourceRect.w;
    moveY %= sourceRect.h;

    if (moveX == 0 && moveY == 0) {
        memcpy(target, source, sourceRect.w * sourceRect.h * 4);
        return true;
    }

    /*

-- VERTICAL --
move 5 pixels

8 pixels high

 source                target
0 ########## ←-*    0 ----------- | wrap here
1 ##########   |    1 -----------
2 ##########   |    2 -----------
3 ##########   |    3 -----------
4 ##########   |    4 -----------
5 ##########   *----5 ----------- | start here
6 ##########        6 ----------- |
7 ##########        7 ----------- |

-- HORIZONTAL --

10 pixels wide

move 2 pixels

       0123456789
source 1111111122
       ↑
       *-*
         |
target 2211111111

---

1. copy 8 pixels (width - offset)
2. copy 2 pixels (offset) to width - offset

    */

    int firstWidth = sourceRect.w - moveX;

    for (int y = 0; y < sourceRect.h; y++) {
        int targetPixelIndex = (sourceRect.w * ((y + moveY) % sourceRect.h));
        int sourcePixelIndex = (sourceRect.w * y);

        memcpy(target + targetPixelIndex + moveX, source + sourcePixelIndex,
               firstWidth * 4);

        memcpy(target + targetPixelIndex,
               source + sourcePixelIndex + firstWidth, moveX * 4);
    }

    return true;
}
