#include "timeline.hpp"
#include <QApplication>
#include <QEasingCurve>
#include <QPainter>
#include <QPushButton>
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

    timelineLeftScrollArea = new QScrollArea(splitter);
    timelineLeftScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    timelineLeftScrollArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    timelineLeftScrollArea->verticalScrollBar()->setEnabled(false);
    timelineLeftContents = new QWidget(timelineLeftScrollArea);
    timelineLeftLayout = new QVBoxLayout(timelineLeftContents);
    timelineLeftLayout->setContentsMargins(0, 0, 0, 0);
    timelineLeftLayout->setSpacing(0);
    timelineLeftScrollArea->setWidget(timelineLeftContents);
    timelineLeftScrollArea->setWidgetResizable(true);
    splitter->addWidget(timelineLeftScrollArea);

    timelineMainScrollArea = new QScrollArea(splitter);
    timelineMainScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    timelineMainScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    connect(timelineMainScrollArea->verticalScrollBar(),
            &QScrollBar::valueChanged, this, &TimelineWidget::timelineScrolled);
    timelineContent = new TimelineContentWidget(this, this);
    timelineMainScrollArea->setWidget(timelineContent);
    splitter->addWidget(timelineMainScrollArea);

    splitter->setSizes({250, 900});

    lay->addWidget(splitter);
}

void TimelineWidget::updateContents() {
    while (timelineLeftLayout->count() > 0) {
        auto item = timelineLeftLayout->takeAt(0);
        QWidget *widget = item->widget();
        if (widget != nullptr) {
            widget->setParent(nullptr);
            widget->deleteLater();
        }
        delete item;
    }

    timelineLeftLayout->addSpacing(TIMELINE_HEADER_HEIGHT);

    for (auto element : scene->elements) {
        bool selected = scene->selectedElements.contains(element);
        QPushButton *elementButton = new QPushButton(timelineLeftContents);
        elementButton->setStyleSheet("QPushButton {"
                                     "   text-align: left;"
                                     "   padding-left: 8px;"
                                     "   background: transparent;"
                                     "   border-radius: 0px;"
                                     "}"
                                     "QPushButton:hover {"
                                     "   background: rgba(128,128,128,0.1);"
                                     "}"
                                     "QPushButton[flat=\"false\"] {"
                                     "   background: rgba(128,128,128,0.25);"
                                     "}"
                                     "QPushButton:pressed {"
                                     "   background: rgba(128,128,128,0.3);"
                                     "}");
        elementButton->setText(QString::fromStdString(element->name));
        elementButton->setFixedHeight(OBJECT_TRACK_HEIGHT);
        elementButton->setFlat(!selected);
        connect(elementButton, &QPushButton::clicked, this, [this, element]() {
            auto modifiers = QApplication::queryKeyboardModifiers();
            // TODO: Shift should select range
            if (modifiers.testFlag(Qt::ControlModifier) ||
                modifiers.testFlag(Qt::ShiftModifier)) {
                QList<Element *> newSelected = scene->selectedElements;
                if (newSelected.contains(element)) {
                    newSelected.removeOne(element);
                } else {
                    newSelected.append(element);
                }
                scene->selectElements(newSelected);
            } else {
                scene->selectElements({element});
            }
        });
        timelineLeftLayout->addWidget(elementButton);
    }

    timelineLeftLayout->addSpacing(64);
    timelineLeftLayout->addStretch();
    timelineLeftScrollArea->verticalScrollBar()->setValue(
        timelineMainScrollArea->verticalScrollBar()->value());

    timelineContent->updateContents();
}

void TimelineWidget::timelineScrolled() {
    timelineLeftScrollArea->verticalScrollBar()->setValue(
        timelineMainScrollArea->verticalScrollBar()->value());

    timelineContent->updateContents();
}

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

    this->setFixedSize(secondsToPixels(5) + TIMELINE_START_OFFSET * 2, height);
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

    double startOffset = TIMELINE_START_OFFSET;
    painter.translate(startOffset, 0);

    double curSec = 2;

    double yPos = 0;
    yPos += TIMELINE_HEADER_HEIGHT;

    // --- Elements ---
    bool stripe = false;
    for (auto element : timelineWidget->scene->elements) {
        if (stripe) {
            painter.fillRect(-startOffset, yPos, width(), OBJECT_TRACK_HEIGHT,
                             palette().alternateBase());
        }
        stripe = !stripe;
        yPos += OBJECT_TRACK_HEIGHT;

        // if (element->collapsed)
        //     continue;

        // --- Properties ---
    }

    // --- Timeline Header ---
    double headerPos =
        timelineWidget->timelineMainScrollArea->verticalScrollBar()->value();
    painter.fillRect(-startOffset, headerPos, width(), TIMELINE_HEADER_HEIGHT,
                     palette().window());

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
