#include "path_element.hpp"
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

PathElement::PathElement() : Element() {
    w.hidden = true;
    h.hidden = true;

    strokeWidth.setMin(0);
    from.setMin(0);
    from.setMax(100);
    to.setMin(0);
    to.setMax(100);

    cap.enumList.push_back("Square");
    cap.enumList.push_back("Flat");
    cap.enumList.push_back("Round");
    cap.updateBoundsToEnumList();

    join.enumList.push_back("Bevel");
    join.enumList.push_back("Miter");
    join.enumList.push_back("Round");
    join.updateBoundsToEnumList();
}

AnimatableRender *PathElement::createClass() { return new PathElementRender(); }

QRect PathElement::getRawBoundingBox(const FrameInfo &frameInfo) {
    PathElementRender *renderElement = (PathElementRender *)toRender(frameInfo);
    renderElement->prepare();
    QRect renderBox = renderElement->rect;
    renderBox.translate(renderElement->x, renderElement->y);
    delete renderElement;

    return renderBox;
}

void PathElementRender::prepare() {
    Path &path = this->path;
    if (!path.points.isEmpty()) {
        painterPath.moveTo(path.points[0].x, path.points[0].y);
    }

    for (int i = 1; i < path.points.length(); i++) {
        auto &point = path.points[i];
        auto &lastPoint = path.points[i - 1];
        QPoint actualPoint = QPoint(point.x, point.y);
        QPoint c1 = QPoint(lastPoint.x + lastPoint.curveX,
                           lastPoint.y + lastPoint.curveY);
        QPoint c2 = QPoint(point.x - point.curveX, point.y - point.curveY);
        painterPath.cubicTo(c1, c2, actualPoint);
    }

    if (path.closed) {
        auto &firstPoint = path.points.first();
        auto &lastPoint = path.points.last();
        painterPath.cubicTo(
            lastPoint.x + lastPoint.curveX, lastPoint.y + lastPoint.curveY,
            firstPoint.x - firstPoint.curveX, firstPoint.y - firstPoint.curveY,
            firstPoint.x, firstPoint.y);
    }

    rect = painterPath.boundingRect().toRect().adjusted(
        -strokeWidth / 2 - 1, -strokeWidth / 2 - 1, strokeWidth / 2 + 1,
        strokeWidth / 2 + 1);
}

Rect PathElementRender::getRenderBox() {
    return Rect::fromQRect(rect.translated(x, y));
}

bool PathElementRender::render(uint32_t *target) {
    QImage img(rect.width(), rect.height(), QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);
    if (strokeWidth > 0) {
        static const Qt::PenCapStyle caps[] = {Qt::PenCapStyle::SquareCap,
                                               Qt::PenCapStyle::FlatCap,
                                               Qt::PenCapStyle::RoundCap};
        static const Qt::PenJoinStyle joins[] = {Qt::PenJoinStyle::BevelJoin,
                                                 Qt::PenJoinStyle::MiterJoin,
                                                 Qt::PenJoinStyle::RoundJoin};

        painter.setPen(QPen(stroke.get().toQBrush(rect), strokeWidth,
                            Qt::PenStyle::SolidLine, caps[this->cap.get()],
                            joins[this->join.get()]));
    } else {
        painter.setPen(Qt::NoPen);
    }
    Brush fill = this->fill;
    if (fill.brushType != Brush::SingleColor || fill.color1.a != 0) {
        painter.setBrush(fill.toQBrush(rect));
    } else {
        painter.setBrush(Qt::NoBrush);
    }

    painter.translate(-rect.x(), -rect.y());

    QPainterPath finalPainterPath = painterPath;
    double from = this->from.get() / 100.;
    double to = this->to.get() / 100.;
    double offset = this->offset.get() / 100.;
    if (from != 0 || to != 1 || offset != 0) {
        finalPainterPath = finalPainterPath.trimmed(from, to, offset);
    }

    painter.drawPath(finalPainterPath);

    memcpy(target, img.bits(), rect.width() * rect.height() * 4);

    return true;
}
