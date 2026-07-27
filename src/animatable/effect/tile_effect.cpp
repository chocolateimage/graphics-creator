#include "tile_effect.hpp"
#include "math.hpp"

TileEffect::TileEffect() {
    multiplyX.setMin(0);
    multiplyY.setMin(0);

    shift.suffix = "%";

    shiftDirection.enumList.push_back("Vertical");
    shiftDirection.enumList.push_back("Horizontal");
    shiftDirection.updateBoundsToEnumList();
}

Rect TileEffectRender::getRenderBox(const Rect &lastBox) {
    int newW = std::clamp((int)(lastBox.w * multiplyX), 1, 10000);
    int newH = std::clamp((int)(lastBox.h * multiplyY), 1, 10000);
    return {lastBox.x + lastBox.w / 2 - newW / 2,
            lastBox.y + lastBox.h / 2 - newH / 2, newW, newH};
}

bool TileEffectRender::render(const uint32_t *source, const Rect &sourceRect,
                              uint32_t *target) {
    Rect rect = renderBox;
    int ox = rect.x - sourceRect.x;
    int oy = rect.y - sourceRect.y;
    bool mirrorOdd = this->mirrorOdd;
    bool showOriginal = this->showOriginal;
    int shiftDirection = this->shiftDirection;
    float shift = this->shift / 100.;
    int shiftX = 0;
    int shiftY = 0;
    if (shiftDirection == 0) {
        shiftY = sourceRect.h * shift;
    } else {
        shiftX = sourceRect.w * shift;
    }
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            int sx = x + ox;
            int sy = y + oy;

            int tileX = floorDiv(sx, sourceRect.w);
            int tileY = floorDiv(sy, sourceRect.h);
            bool isOddX{false};
            bool isOddY{false};

            if (!showOriginal && tileX == 0 && tileY == 0) {
                continue;
            }

            isOddX = mod(tileX, 2) == 1;
            isOddY = mod(tileY, 2) == 1;

            if (shiftDirection == 0 ? isOddX : isOddY) {
                sx -= shiftX;
                sy -= shiftY;
                tileX = floorDiv(sx, sourceRect.w);
                tileY = floorDiv(sy, sourceRect.h);
                isOddX = mod(tileX, 2) == 1;
                isOddY = mod(tileY, 2) == 1;
            }

            sx = mod(sx, sourceRect.w);
            sy = mod(sy, sourceRect.h);

            if (mirrorOdd) {
                if (isOddX) {
                    sx = sourceRect.w - sx - 1;
                }
                if (isOddY) {
                    sy = sourceRect.h - sy - 1;
                }
            }

            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(sx, sy, sourceRect.w)];
        }
    }

    return true;
}
