#include "image_element.hpp"
#include "math.hpp"
#include "render.hpp"
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

ImageElement::ImageElement() : Element() {}

AnimatableRender *ImageElement::createClass() {
    return new ImageElementRender();
}

bool ImageElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    int w = this->w;
    int h = this->h;
    bool scaled = this->scaled;

    QString path = QString::fromStdString(this->path);
    if (scaled && path.endsWith(".svg")) {
        QSvgRenderer renderer(path);
        QImage image(w, h, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);

        renderer.setAnimationEnabled(true);
        renderer.setFramesPerSecond(30);
        renderer.setCurrentFrame(currentSeconds * 30);
        renderer.render(&painter);

        memcpy(target, image.bits(), w * h * 4);

        return true;
    }

    auto imageData = globalImageLoader.loadImage(this->path);
    int iw = imageData->width;
    int ih = imageData->height;
    int tw = std::min(w, imageData->width);
    int th = std::min(h, imageData->height);

    if (scaled) {
        if (w == iw && h == ih) {
            scaled = false;
        }
        tw = w;
        th = h;
    }

    for (int y = 0; y < th; y++) {
        for (int x = 0; x < tw; x++) {
            int sx = scaled ? (int)((float)x / w * iw) : x;
            int sy = scaled ? (int)((float)y / h * ih) : y;
            target[pixelIndex(x, y, rect.w)] =
                imageData->data[pixelIndex(sx, sy, iw)];
        }
    }

    return true;
}
