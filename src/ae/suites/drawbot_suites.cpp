#include "drawbot_suites.hpp"
#include "gui/ae_custom_ui.hpp"
#include "variant.hpp"
#include <QApplication>
#include <QBrush>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <qnamespace.h>

class DrawbotResource {
  public:
    DrawbotResource() {}
    virtual ~DrawbotResource() {}

    void retain() { referenceCount++; }
    bool release() {
        referenceCount--;
        return referenceCount == 0;
    }

  private:
    int referenceCount{1};
};

class DrawbotPath : public DrawbotResource {
  public:
    DrawbotPath() {}
    ~DrawbotPath() {}

    QPainterPath path{};
};

class DrawbotBrush : public DrawbotResource {
  public:
    DrawbotBrush(RGBAf *rgba) {
        brush = QBrush(QColor(rgba->r * 255., rgba->g * 255., rgba->b * 255.,
                              rgba->a * 255.));
    }
    ~DrawbotBrush() {}

    QBrush brush;
};

class DrawbotPen : public DrawbotResource {
  public:
    DrawbotPen(RGBAf *rgba, float size) {
        pen = QPen(QColor(rgba->r * 255., rgba->g * 255., rgba->b * 255.,
                          rgba->a * 255.),
                   size);
    }
    ~DrawbotPen() {}

    QPen pen;
};

class DrawbotFont : public DrawbotResource {
  public:
    DrawbotFont() {}
    ~DrawbotFont() {}

    QFont font{};
};

int32_t getSupplier(void *drawingRef, void **supplierRef) {
    *supplierRef = drawingRef;
    return 0;
}

int32_t getSurface(void *drawingRef, void **surfaceRef) {
    *surfaceRef = drawingRef;
    return 0;
}

int32_t newPen(void *, RGBAf *rgba, float size, DrawbotPen **pen) {
    *pen = new DrawbotPen(rgba, size);
    return 0;
}

int32_t newBrush(void *, RGBAf *rgba, DrawbotBrush **brush) {
    *brush = new DrawbotBrush(rgba);
    return 0;
}

int32_t supportsText(void *, bool *value) {
    *value = true;
    return 0;
}
int32_t getDefaultFontSize(void *, float *fontSize) {
    if (qApp->font().pixelSize() == -1) {
        // TODO: Some kind of points to pixels/or in reverse translation
        *fontSize = qApp->font().pointSizeF();
    } else {
        *fontSize = qApp->font().pixelSize();
    }
    return 0;
}
int32_t newDefaultFont(void *, float fontSize, DrawbotFont **font) {
    DrawbotFont *newFont = new DrawbotFont();
    newFont->font.setPointSize(fontSize);
    *font = newFont;
    return 0;
}
int32_t newImageFromBuffer(void *) {
    qInfo() << "newImageFromBuffer";
    return 0;
}
int32_t newPath(void *, DrawbotPath **path) {
    *path = new DrawbotPath();
    return 0;
}
int32_t supportsPixelLayoutBGRA(void *, bool *value) {
    *value = true;
    return 0;
}
int32_t prefersPixelLayoutBGRA(void *, bool *value) {
    *value = true;
    return 0;
}
int32_t supportsPixelLayoutARGB(void *, bool *value) {
    *value = true;
    return 0;
}
int32_t prefersPixelLayoutARGB(void *, bool *value) {
    *value = false;
    return 0;
}
int32_t retainObject(DrawbotResource *resource) {
    resource->retain();
    return 0;
}
int32_t releaseObject(DrawbotResource *resource) {
    if (resource->release()) {
        delete resource;
    }
    return 0;
}
int32_t pushStateStack(void *) {
    qInfo() << "pushStateStack";
    return 0;
}
int32_t popStateStack(void *) {
    qInfo() << "popStateStack";
    return 0;
}
int32_t paintRect(AECustomUIWindow *window, RGBAf *rgba, Rectf *rect) {
    window->painter->setBrush(
        QColor(rgba->r * 255., rgba->g * 255., rgba->b * 255., rgba->a * 255.));
    window->painter->setPen(Qt::NoPen);
    window->painter->drawRect(rect->x, rect->y, rect->w, rect->h);
    return 0;
}
int32_t fillPath(AECustomUIWindow *window, DrawbotBrush *brush,
                 DrawbotPath *path, int fillType) {
    if (fillType == 0) {
        path->path.setFillRule(Qt::FillRule::OddEvenFill);
    } else {
        path->path.setFillRule(Qt::FillRule::WindingFill);
    }
    window->painter->setBrush(brush->brush);
    window->painter->setPen(Qt::NoPen);
    window->painter->drawPath(path->path);
    return 0;
}
int32_t strokePath(AECustomUIWindow *window, DrawbotPen *pen,
                   DrawbotPath *path) {
    window->painter->setBrush(Qt::NoBrush);
    window->painter->setPen(pen->pen);
    window->painter->drawPath(path->path);
    return 0;
}
int32_t clip(void *) {
    qInfo() << "clip";
    return 0;
}
int32_t getClipBounds(void *) {
    qInfo() << "getClipBounds";
    return 0;
}
int32_t isWithinClipBounds(void *) {
    qInfo() << "isWithinClipBounds";
    return 0;
}
int32_t transform(void *) {
    qInfo() << "transform";
    return 0;
}

int32_t drawString(AECustomUIWindow *window, DrawbotBrush *brush,
                   DrawbotFont *font, wchar_t *string, Vector2DFloat *origin,
                   int alignment, int truncation, float width) {
    QPointF point(origin->x, origin->y);
    QString str = QString::fromWCharArray(string);
    window->painter->setBrush(Qt::NoBrush);
    window->painter->setPen(QPen(brush->brush.color()));
    window->painter->setFont(font->font);
    window->painter->drawText(
        QRectF(origin->x, origin->y - font->font.pointSizeF(), 1000, 1000),
        str);
    return 0;
}
int32_t drawImage(void *) {
    qInfo() << "drawImage";
    return 0;
}
int32_t setInterpolationPolicy(void *) {
    qInfo() << "setInterpolationPolicy";
    return 0;
}
int32_t getInterpolationPolicy(void *) {
    qInfo() << "getInterpolationPolicy";
    return 0;
}
int32_t setAntialiasPolicy(void *) {
    qInfo() << "setAntialiasPolicy";
    return 0;
}
int32_t getAntialiasPolicy(void *) {
    qInfo() << "getAntialiasPolicy";
    return 0;
}
int32_t flush(void *) {
    qInfo() << "flush";
    return 0;
}
int32_t getTransformToScreenScale(void *) {
    qInfo() << "getTransformToScreenScale";
    return 0;
}
int32_t moveTo(void *) {
    qInfo() << "moveTo";
    return 0;
}
int32_t lineTo(void *) {
    qInfo() << "lineTo";
    return 0;
}
int32_t bezierTo(void *) {
    qInfo() << "bezierTo";
    return 0;
}
int32_t addRect(DrawbotPath *path, Rectf *rect) {
    path->path.addRect(rect->x, rect->y, rect->w, rect->h);
    return 0;
}
int32_t addArc(void *) {
    qInfo() << "addArc";
    return 0;
}
int32_t drawbotPathClose(void *) {
    qInfo() << "close";
    return 0;
}

DrawbotDrawSuite1::DrawbotDrawSuite1() {
    getSupplier = (void *)::getSupplier;
    getSurface = (void *)::getSurface;
}
DrawbotSupplierSuite1::DrawbotSupplierSuite1() {
    newPen = (void *)::newPen;
    newBrush = (void *)::newBrush;
    supportsText = (void *)::supportsText;
    getDefaultFontSize = (void *)::getDefaultFontSize;
    newDefaultFont = (void *)::newDefaultFont;
    newImageFromBuffer = (void *)::newImageFromBuffer;
    newPath = (void *)::newPath;
    supportsPixelLayoutBGRA = (void *)::supportsPixelLayoutBGRA;
    prefersPixelLayoutBGRA = (void *)::prefersPixelLayoutBGRA;
    supportsPixelLayoutARGB = (void *)::supportsPixelLayoutARGB;
    prefersPixelLayoutARGB = (void *)::prefersPixelLayoutARGB;
    retainObject = (void *)::retainObject;
    releaseObject = (void *)::releaseObject;
}
DrawbotSurfaceSuite2::DrawbotSurfaceSuite2() {
    pushStateStack = (void *)::pushStateStack;
    popStateStack = (void *)::popStateStack;
    paintRect = (void *)::paintRect;
    fillPath = (void *)::fillPath;
    strokePath = (void *)::strokePath;
    clip = (void *)::clip;
    getClipBounds = (void *)::getClipBounds;
    isWithinClipBounds = (void *)::isWithinClipBounds;
    transform = (void *)::transform;
    drawString = (void *)::drawString;
    drawImage = (void *)::drawImage;
    setInterpolationPolicy = (void *)::setInterpolationPolicy;
    getInterpolationPolicy = (void *)::getInterpolationPolicy;
    setAntialiasPolicy = (void *)::setAntialiasPolicy;
    getAntialiasPolicy = (void *)::getAntialiasPolicy;
    flush = (void *)::flush;
    getTransformToScreenScale = (void *)::getTransformToScreenScale;
}
DrawbotPathSuite1::DrawbotPathSuite1() {
    moveTo = (void *)::moveTo;
    lineTo = (void *)::lineTo;
    bezierTo = (void *)::bezierTo;
    addRect = (void *)::addRect;
    addArc = (void *)::addArc;
    close = (void *)::drawbotPathClose;
}
