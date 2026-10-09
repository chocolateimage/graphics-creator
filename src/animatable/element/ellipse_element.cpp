#include "ellipse_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"

EllipseElement::EllipseElement() : Element() { strokeWidth.setMin(0); }

AnimatableRender *EllipseElement::createClass() {
    return new EllipseElementRender();
}

Rect EllipseElementRender::getRenderBox() {
    return {x - strokeWidth, y - strokeWidth, w + strokeWidth * 2,
            h + strokeWidth * 2};
}

bool EllipseElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    int w = this->w;
    int h = this->h;
    int fullW = rect.w;
    int fullH = rect.h;
    int strokeWidth = this->strokeWidth;
    int offsetX = strokeWidth;
    int offsetY = strokeWidth;

    float stepDistance = std::min(1. / w, 1. / h);

    for (int y = 0; y < fullH; y++) {
        for (int x = 0; x < fullW; x++) {
            Color fillColor =
                getBrushPixel(fill.get(), x - offsetX, y - offsetY, w, h);
            Color strokeColor = strokeWidth > 0
                                    ? getBrushPixel(stroke.get(), x, y, w, h)
                                    : fillColor;
            float dist = distance((x + 1. - offsetX) / (w + 1),
                                  (y + 1. - offsetY) / (h + 1), .5, .5);

            float distStroke = distance((x + 1.) / (fullW + 1),
                                        (y + 1.) / (fullH + 1), .5, .5);

            Color c = lerp(fillColor, strokeColor,
                           linearstep(.5 - stepDistance, .5, dist));
            c.a *= linearstep(.5, .5 - stepDistance, distStroke);

            target[pixelIndex(x, y, rect.w)] = makePixel(c);
        }
    }

    return true;
}
