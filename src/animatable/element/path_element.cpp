#include "path_element.hpp"
#include "math.hpp"
#include "render.hpp"
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

PathElement::PathElement() : Element() {
    w.hidden = true;
    h.hidden = true;
}

AnimatableRender *PathElement::createClass() { return new PathElementRender(); }

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

    rect = painterPath.boundingRect().toRect().adjusted(-8, -8, 16, 16);
}

Rect PathElementRender::getRenderBox() { return Rect::fromQRect(rect); }

bool PathElementRender::render(uint32_t *target) {
    QImage img(rect.width(), rect.height(), QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(255, 255, 255), 8));
    painter.translate(-rect.x(), -rect.y());
    painter.drawPath(painterPath);

    memcpy(target, img.bits(), rect.width() * rect.height() * 4);

    return true;
}
