#include "timeline.hpp"
#include <QEasingCurve>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>
#include <QVBoxLayout>

TimelineWidget::TimelineWidget(Scene *scene, QWidget *parent)
    : QWidget(parent) {
    this->scene = scene;

    connect(scene, &Scene::elementAdded, this, &TimelineWidget::updateContents);
    connect(scene, &Scene::elementRemoved, this,
            &TimelineWidget::updateContents);
    connect(scene, &Scene::elementSelectionChanged, this,
            &TimelineWidget::updateContents);

    auto lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(this);

    QScrollArea *timelineLeftScrollArea = new QScrollArea(splitter);
    timelineLeftScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    timelineLeftScrollArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    timelineLeftScrollArea->verticalScrollBar()->setEnabled(false);
    QWidget *timelineLeftContents = new QWidget(timelineLeftScrollArea);
    QVBoxLayout *timelineLeftLayout = new QVBoxLayout(timelineLeftContents);
    timelineLeftScrollArea->setWidget(timelineLeftContents);
    timelineLeftScrollArea->setWidgetResizable(true);
    splitter->addWidget(timelineLeftScrollArea);

    QScrollArea *timelineMainScrollArea = new QScrollArea(splitter);
    timelineMainScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineMainScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineContent = new TimelineContentWidget(this, this);
    timelineMainScrollArea->setWidget(timelineContent);
    splitter->addWidget(timelineMainScrollArea);

    splitter->setSizes({250, 900});

    lay->addWidget(splitter);
}

void TimelineWidget::updateContents() { timelineContent->updateContents(); }

TimelineContentWidget::TimelineContentWidget(TimelineWidget *timelineWidget,
                                             QWidget *parent)
    : QWidget(parent), timelineWidget(timelineWidget) {
    new QVBoxLayout(this);
    setMouseTracking(true);
    updateContents();
}

void TimelineContentWidget::updateContents() {
    double height = TIMELINE_HEADER_HEIGHT;
    for (auto elements : timelineWidget->scene->elements) {
        height += OBJECT_TRACK_HEIGHT;
        // height += PROPERTY_TRACK_HEIGHT * elements->propertyTracks.size();
    }

    this->setFixedSize(secondsToPixels(5), height);
    update();
}

double TimelineContentWidget::secondsToPixels() { return 30; }

double TimelineContentWidget::secondsToPixels(double seconds) {
    return seconds * secondsToPixels();
}

void TimelineContentWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    double curSec = 2;

    double yPos = 0;
    yPos += TIMELINE_HEADER_HEIGHT;

    // --- Elements ---
    bool stripe = false;
    for (auto element : timelineWidget->scene->elements) {
        if (stripe) {
            painter.fillRect(0, yPos, width(), OBJECT_TRACK_HEIGHT,
                             palette().alternateBase());
        }
        stripe = !stripe;
        yPos += OBJECT_TRACK_HEIGHT;

        // if (element->collapsed)
        //     continue;

        // --- Properties ---
    }

    // --- Timeline Header ---
    double headerPos = 0;
    painter.fillRect(0, headerPos, width(), TIMELINE_HEADER_HEIGHT,
                     palette().base());

    // Seconds Text
    painter.setPen(QPen(palette().placeholderText(), 2));
    QFontMetrics metrics(painter.font());
    for (int x = 0; x <= width(); x += secondsToPixels()) {
        int second = x / secondsToPixels();
        QString text = QString::number(second);
        painter.drawText(x - 64, headerPos, 128, TIMELINE_HEADER_HEIGHT,
                         Qt::AlignCenter, text);
    }

    // Position Marker
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().accent());
    int markerWidth = 16;
    QPolygonF polygon;
    polygon << QPointF(secondsToPixels(curSec) - (markerWidth / 2.0),
                       headerPos);
    polygon << QPointF(secondsToPixels(curSec) + (markerWidth / 2.0),
                       headerPos);
    polygon << QPointF(secondsToPixels(curSec) + (markerWidth / 2.0),
                       headerPos + TIMELINE_HEADER_HEIGHT / 2.0);
    polygon << QPointF(secondsToPixels(curSec),
                       headerPos + TIMELINE_HEADER_HEIGHT);
    polygon << QPointF(secondsToPixels(curSec) - (markerWidth / 2.0),
                       headerPos + TIMELINE_HEADER_HEIGHT / 2.0);
    painter.drawPolygon(polygon);

    painter.setPen(QPen(palette().accent(), 2));
    painter.drawLine(secondsToPixels(curSec), headerPos,
                     secondsToPixels(curSec), height());
}

void TimelineContentWidget::mousePressEvent(QMouseEvent *event) {}

void TimelineContentWidget::mouseMoveEvent(QMouseEvent *event) {}

void TimelineContentWidget::mouseReleaseEvent(QMouseEvent *event) {}
