#include "timeline.hpp"
#include <QApplication>
#include <QEasingCurve>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpacerItem>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

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
    timelineLeftContents->setMinimumHeight(100000);
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

    timelineLeftContents->installEventFilter(this);
}

bool TimelineWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == timelineLeftContents) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *oldEvent = (QWheelEvent *)event;
            QWheelEvent *newEvent = new QWheelEvent(
                oldEvent->position(), oldEvent->globalPosition(),
                oldEvent->pixelDelta(), oldEvent->angleDelta(),
                oldEvent->buttons(), oldEvent->modifiers(), oldEvent->phase(),
                oldEvent->inverted(), oldEvent->source(),
                oldEvent->pointingDevice());
            QApplication::sendEvent(timelineMainScrollArea->verticalScrollBar(),
                                    newEvent);
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void TimelineWidget::updateContents() {
    while (timelineLeftLayout->count() > 0) {
        auto item = timelineLeftLayout->takeAt(0);
        QWidget *widget = item->widget();
        if (widget != nullptr) {
            widget->hide();
            widget->setParent(nullptr);
            widget->deleteLater();
        }
        delete item;
    }

    timelineLeftLayout->addSpacing(TIMELINE_HEADER_HEIGHT);

    bool stripe = false;
    for (auto element : scene->elements) {
        disconnect(element, &Element::propertyIsAnimatingUpdated, this,
                   &TimelineWidget::updateContents);
        connect(element, &Element::propertyIsAnimatingUpdated, this,
                &TimelineWidget::updateContents);
        bool selected = scene->selectedElements.contains(element);
        QPushButton *elementButton = new QPushButton(timelineLeftContents);
        elementButton->setStyleSheet(
            "QPushButton {"
            "   text-align: left;"
            "   padding-left: 8px;"
            "   background: transparent;"
            "   border-radius: 0px;"
            "   font-weight: 600;"
            "}"
            "QPushButton:hover {"
            "   background: rgba(128,128,128,0.1);"
            "}"
            "QPushButton[flat=\"false\"] {"
            "   background: rgba(128,128,128,0.25);"
            "   border-left: 3px solid palette(accent);"
            "   padding-left: 5px;"
            "}"
            "QPushButton:pressed {"
            "   background: rgba(128,128,128,0.3);"
            "}");
        if (element->collapsed) {
            elementButton->setIcon(QIcon::fromTheme("arrow-right"));
        } else {
            elementButton->setIcon(QIcon::fromTheme("arrow-down"));
        }
        elementButton->setText(element->objectName());
        elementButton->setFixedHeight(OBJECT_TRACK_HEIGHT);
        elementButton->setFlat(!selected);
        connect(elementButton, &QPushButton::clicked, this,
                [this, element, elementButton]() {
                    QPoint pos = elementButton->mapFromGlobal(QCursor::pos());
                    bool isInsideCollapsedButton =
                        pos.y() < OBJECT_TRACK_HEIGHT && pos.x() < 32;
                    if (isInsideCollapsedButton) {
                        element->collapsed = !element->collapsed;
                        QTimer::singleShot(0, this,
                                           &TimelineWidget::updateContents);
                        return;
                    }

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
        connect(element, &Element::objectNameChanged, elementButton,
                [elementButton](const QString &objectName) {
                    elementButton->setText(objectName);
                });
        timelineLeftLayout->addWidget(elementButton);

        stripe = !stripe;

        if (!element->collapsed) {
            for (auto property : element->properties) {
                if (!property->isAnimatable())
                    continue;

                QPushButton *propertyButton =
                    new QPushButton(timelineLeftContents);
                propertyButton->setObjectName("property");
                QString background = "transparent";
                QString backgroundSelected = "rgba(128,128,128,0.1)";
                if (stripe) {
                    background = "palette(alternate-base)";
                    backgroundSelected = "rgba(128,128,128,0.13)";
                }
                propertyButton->setStyleSheet(
                    "#property {"
                    "   text-align: left;"
                    "   background: " +
                    background +
                    ";"
                    "   border-radius: 0px;"
                    "}"
                    "#property[flat=\"false\"] {"
                    "   background: " +
                    backgroundSelected +
                    ";"
                    "   border-left: 3px solid palette(accent);"
                    "}");
                propertyButton->setFixedHeight(PROPERTY_TRACK_HEIGHT);
                propertyButton->setFlat(!selected);
                connect(propertyButton, &QPushButton::clicked, elementButton,
                        &QPushButton::click);

                QHBoxLayout *propertyLayout = new QHBoxLayout(propertyButton);
                propertyLayout->setContentsMargins(24, 0, 0, 0);

                QPushButton *toggleAnimationButton =
                    new QPushButton(propertyButton);
                if (property->isAnimating) {
                    toggleAnimationButton->setIcon(
                        QIcon::fromTheme("keyframe"));
                    toggleAnimationButton->setToolTip("Animation enabled");
                } else {
                    toggleAnimationButton->setIcon(
                        QIcon::fromTheme("keyframe-disable"));
                    toggleAnimationButton->setToolTip("Animation disabled");
                }
                toggleAnimationButton->setFlat(true);
                toggleAnimationButton->setFixedWidth(32);
                connect(toggleAnimationButton, &QPushButton::clicked, this,
                        [property]() { property->toggleAnimating(); });
                propertyLayout->addWidget(toggleAnimationButton);

                QLabel *label = new QLabel(propertyButton);
                label->setText(
                    QString::fromStdString(property->getDisplayName()));
                propertyLayout->addWidget(label);

                timelineLeftLayout->addWidget(propertyButton);
                stripe = !stripe;
            }
        }
    }

    timelineLeftLayout->addSpacing(64);
    timelineLeftLayout->addStretch();

    timelineScrolled();
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
    for (auto element : timelineWidget->scene->elements) {
        height += OBJECT_TRACK_HEIGHT;

        if (element->collapsed)
            continue;
        for (auto property : element->properties) {
            if (!property->isAnimatable())
                continue;

            height += PROPERTY_TRACK_HEIGHT;
        }
    }

    this->setFixedSize(secondsToPixels(timelineWidget->scene->durationFrames /
                                       timelineWidget->scene->frameRate) +
                           TIMELINE_START_OFFSET * 2,
                       height);
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

        if (element->collapsed)
            continue;

        // --- Properties ---
        for (auto property : element->properties) {
            if (!property->isAnimatable())
                continue;

            if (stripe) {
                painter.fillRect(-startOffset, yPos, width(),
                                 PROPERTY_TRACK_HEIGHT,
                                 palette().alternateBase());
            }
            stripe = !stripe;

            if (property->isAnimating) {
                for (auto keyframe : property->keyframes) {
                    // Diamonds

                    painter.setBrush(QColor(180, 180, 180));
                    painter.setPen(Qt::NoPen);

                    double kXPos = secondsToPixels(
                        keyframe->frame * timelineWidget->scene->frameRate);
                    double kWidth = 10;
                    double kYPos =
                        yPos + PROPERTY_TRACK_HEIGHT / 2.0 - kWidth / 2.0;

                    QPolygonF polygon;
                    polygon << QPointF(kXPos, kYPos);
                    polygon
                        << QPointF(kXPos + kWidth / 2.0, kYPos + kWidth / 2.0);
                    polygon << QPointF(kXPos, kYPos + kWidth);
                    polygon
                        << QPointF(kXPos - kWidth / 2.0, kYPos + kWidth / 2.0);
                    painter.drawPolygon(polygon);
                }
            }

            yPos += PROPERTY_TRACK_HEIGHT;
        }
    }

    // --- Timeline Header ---
    double headerPos =
        timelineWidget->timelineMainScrollArea->verticalScrollBar()->value();
    painter.fillRect(-startOffset, headerPos, width(), TIMELINE_HEADER_HEIGHT,
                     palette().window());

    // Seconds Text
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    QFontMetrics metrics(painter.font());
    for (int x = 0; x <= width(); x += secondsToPixels()) {
        int second = x / secondsToPixels();
        QString text = QString::number(second) + "s";
        painter.setPen(QPen(palette().placeholderText(), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawText(x - 64, headerPos, 128, TIMELINE_HEADER_HEIGHT,
                         Qt::AlignHCenter | Qt::AlignTop, text);
        painter.setPen(Qt::NoPen);
        painter.setBrush(palette().placeholderText().color().darker());
        constexpr int lineHeight = 8;
        painter.drawRect(x, headerPos + TIMELINE_HEADER_HEIGHT - lineHeight, 1,
                         lineHeight);
    }

    // Position Marker
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().accent());
    constexpr int markerWidth = 16;
    constexpr int markerHeight = 10;
    int markerY = headerPos + TIMELINE_HEADER_HEIGHT - markerHeight;
    QPolygonF polygon;
    polygon << QPointF(secondsToPixels(curSec) - (markerWidth / 2.0), markerY);
    polygon << QPointF(secondsToPixels(curSec) + (markerWidth / 2.0), markerY);
    polygon << QPointF(secondsToPixels(curSec) + (markerWidth / 2.0),
                       markerY + markerHeight / 2.0);
    polygon << QPointF(secondsToPixels(curSec), markerY + markerHeight);
    polygon << QPointF(secondsToPixels(curSec) - (markerWidth / 2.0),
                       markerY + markerHeight / 2.0);
    painter.drawPolygon(polygon);

    painter.setPen(QPen(palette().accent(), 2));
    painter.drawLine(secondsToPixels(curSec), markerY + 2,
                     secondsToPixels(curSec), height());
}

void TimelineContentWidget::mousePressEvent(QMouseEvent *event) {}

void TimelineContentWidget::mouseMoveEvent(QMouseEvent *event) {}

void TimelineContentWidget::mouseReleaseEvent(QMouseEvent *event) {}
