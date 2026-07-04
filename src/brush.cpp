#include "brush.hpp"

Color getBrushPixel(const Brush &brush, int x, int y, int w, int h) {
    switch (brush.brushType) {
    case Brush::SingleColor:
        return brush.color1;
    default: // TODO: add brush types
        return {0, 0, 0, 0};
    }
}
