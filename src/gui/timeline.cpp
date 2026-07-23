#include "timeline.hpp"
#include "gui.hpp"
#include "line.hpp"
#include <KIconColors>
#include <KIconLoader>
#include <QActionGroup>
#include <QApplication>
#include <QDrag>
#include <QEasingCurve>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpacerItem>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>

const QString ELEMENT_DRAG_MIME_TYPE = "application/x-graphicscreator-element";

TimelineElementButton::TimelineElementButton(Element *element,
                                             TimelineWidget *timelineWidget)
    : QPushButton(timelineWidget->timelineLeftContents),
      timelineWidget(timelineWidget), element(element) {
    bool selected = timelineWidget->scene->selectedElements.contains(element);
    setStyleSheet("QPushButton {"
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
        setIcon(QIcon::fromTheme("arrow-right"));
    } else {
        setIcon(QIcon::fromTheme("arrow-down"));
    }
    setText(element->objectName());
    setFixedHeight(OBJECT_TRACK_HEIGHT);
    setFlat(!selected);

    connect(element, &Element::objectNameChanged, this,
            &TimelineElementButton::elementNameChanged);
    connect(this, &QPushButton::clicked, this,
            &TimelineElementButton::clickedSlot);
}

void TimelineElementButton::elementNameChanged(const QString &objectName) {
    setText(objectName);
}

void TimelineElementButton::clickedSlot() {
    QPoint pos = mapFromGlobal(QCursor::pos());
    bool isInsideCollapsedButton =
        pos.y() < OBJECT_TRACK_HEIGHT && pos.x() < 32;
    if (isInsideCollapsedButton) {
        element->collapsed = !element->collapsed;
        QTimer::singleShot(0, timelineWidget, &TimelineWidget::updateContents);
        return;
    }

    Scene *scene = timelineWidget->scene;
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
}

void TimelineElementButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }

    return QPushButton::mousePressEvent(event);
}

void TimelineElementButton::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton))
        return QPushButton::mouseMoveEvent(event);
    if ((event->pos() - dragStartPosition).manhattanLength() <
        QApplication::startDragDistance())
        return QPushButton::mouseMoveEvent(event);

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData();
    // This is some very cursed code :')
    mimeData->setData(ELEMENT_DRAG_MIME_TYPE,
                      QString::number((uint64_t)(element)).toUtf8());
    drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
}

TimelineWidget::TimelineWidget(Scene *scene, NewMainWindow *mainWindow,
                               QWidget *parent)
    : QWidget(parent), mainWindow(mainWindow), keyframeNo(24, 24),
      keyframeYes(24, 24) {
    this->scene = scene;
    setAcceptDrops(true);

    createPixmaps();

    connect(scene, &Scene::elementAdded, this, &TimelineWidget::updateContents);
    connect(scene, &Scene::elementRemoved, this,
            &TimelineWidget::updateContents);
    connect(scene, &Scene::elementSelectionChanged, this,
            &TimelineWidget::updateContents);
    connect(scene, &Scene::elementOrderChanged, this,
            &TimelineWidget::updateContents);

    auto mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    auto toolbar = new QFrame(this);
    toolbar->setFrameShape(QFrame::Shape::StyledPanel);
    toolbar->setFrameShadow(QFrame::Shadow::Plain);
    QPalette palette = toolbar->palette();
    palette.setColor(QPalette::ColorRole::Base, Qt::red);
    toolbar->setPalette(palette);
    auto toolbarLay = new QHBoxLayout(toolbar);
    toolbarLay->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    toolbarLay->setContentsMargins(12, 0, 12, 0);
    toolbarLay->setSpacing(0);

    QPushButton *goToStartButton = new QPushButton(toolbar);
    goToStartButton->setToolTip("Go to start");
    goToStartButton->setFlat(true);
    goToStartButton->setIcon(QIcon::fromTheme("media-skip-backward"));
    connect(goToStartButton, &QPushButton::clicked, this,
            &TimelineWidget::goToStart);
    toolbarLay->addWidget(goToStartButton);

    playButton = new QPushButton(toolbar);
    playButton->setFlat(true);
    playbackStateChanged(false);
    connect(scene, &Scene::playbackStateChanged, this,
            &TimelineWidget::playbackStateChanged);
    connect(playButton, &QPushButton::clicked, this,
            &TimelineWidget::togglePlay);
    toolbarLay->addWidget(playButton);

    toolbarLay->addStretch();

    zoomSlider = new QSlider(toolbar);
    zoomSlider->setOrientation(Qt::Horizontal);
    zoomSlider->setRange(1, 150);
    zoomSlider->setValue(30);
    zoomSlider->setMaximumWidth(100);
    toolbarLay->addWidget(zoomSlider);

    mainLay->addWidget(toolbar);

    mainLay->addWidget(new HorizontalLine(this));

    auto lay = new QVBoxLayout();
    mainLay->addLayout(lay, 1);
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

    connect(zoomSlider, &QSlider::valueChanged, timelineContent,
            &TimelineContentWidget::updateContents);
}

void TimelineWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat(ELEMENT_DRAG_MIME_TYPE)) {
        event->accept();
    }
}

void TimelineWidget::dragMoveEvent(QDragMoveEvent *event) {
    if (event->mimeData()->hasFormat(ELEMENT_DRAG_MIME_TYPE)) {
        if (!elementMoveBar) {
            elementMoveBar = new QFrame(timelineLeftContents);
            elementMoveBar->setFrameShape(QFrame::Shape::StyledPanel);
            elementMoveBar->setFrameShadow(QFrame::Shadow::Raised);
            elementMoveBar->setFixedWidth(timelineLeftContents->width());
            elementMoveBar->setFixedHeight(2);
            elementMoveBar->setAutoFillBackground(true);
            elementMoveBar->setBackgroundRole(QPalette::ColorRole::Accent);
            timelineLeftLayout->addWidget(elementMoveBar);
        }

        QList<int> yPositions;

        yPositions.append(elementButtons[0]->y());

        for (auto element : elementButtons) {
            yPositions.append(element->y() + OBJECT_TRACK_HEIGHT);
        }

        QPointF pos =
            timelineLeftContents->mapFromGlobal(mapToGlobal(event->position()));
        int yPos = pos.y();
        int targetElementIndex = 0;
        int lastClosest = INT32_MAX;
        for (int i = 0; i < yPositions.size(); i++) {
            int diff = std::abs(yPositions[i] - yPos);
            if (diff < lastClosest) {
                targetElementIndex = i;
                lastClosest = diff;
            }
        }
        elementMoveTarget = targetElementIndex;
        elementMoveBar->move(0, yPositions[targetElementIndex]);
    }
}

void TimelineWidget::dragLeaveEvent(QDragLeaveEvent *event) {
    if (elementMoveBar) {
        elementMoveBar->deleteLater();
        elementMoveBar = nullptr;
    }
}

void TimelineWidget::dropEvent(QDropEvent *event) {
    if (elementMoveBar) {
        uint64_t address =
            QString::fromUtf8(event->mimeData()->data(ELEMENT_DRAG_MIME_TYPE))
                .toULongLong();
        Element *gotElement = (Element *)address;
        scene->reorderElement(gotElement, elementMoveTarget);

        elementMoveBar->deleteLater();
        elementMoveBar = nullptr;
    }
}

void TimelineWidget::createPixmaps() {
    QPolygon polygon;
    int size = 8;
    int offset = 4;
    polygon << QPoint(size, 0);
    polygon << QPoint(size * 2, size);
    polygon << QPoint(size, size * 2);
    polygon << QPoint(0, size);
    keyframeNo.fill(Qt::transparent);
    {
        QPainter painter(&keyframeNo);
        painter.translate(offset, offset);
        painter.setPen(Qt::NoPen);
        painter.setBrush(palette().text());
        painter.drawPolygon(polygon);
    }
    keyframeYes.fill(Qt::transparent);
    {
        QPainter painter(&keyframeYes);
        painter.translate(offset, offset);
        painter.setPen(Qt::NoPen);
        painter.setBrush(palette().accent());
        painter.drawPolygon(polygon);
    }
}

void TimelineWidget::goToStart() {
    if (scene->isPlaying()) {
        scene->stopTimer();
    }
    scene->setFramesChanging(true);
    scene->setFrame(0);
    scene->setFramesChanging(false);
}

void TimelineWidget::togglePlay() {
    if (scene->isPlaying()) {
        scene->stopTimer();
    } else {
        scene->startTimer();
    }
}

void TimelineWidget::playbackStateChanged(bool playing) {
    if (playing) {
        playButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        playButton->setToolTip("Pause");
    } else {
        playButton->setToolTip("Play");
        playButton->setIcon(QIcon::fromTheme("media-playback-start"));
    }
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

    elementButtons.clear();
    bool stripe = false;
    for (auto element : scene->elements) {
        disconnect(element, &Element::effectListUpdated, this,
                   &TimelineWidget::updateContents);
        connect(element, &Element::effectListUpdated, this,
                &TimelineWidget::updateContents);
        disconnect(element, &Element::propertyUpdated, timelineContent,
                   &TimelineContentWidget::updatePaint);
        connect(element, &Element::propertyUpdated, timelineContent,
                &TimelineContentWidget::updatePaint);
        disconnect(element, &Element::effectPropertyUpdated, timelineContent,
                   &TimelineContentWidget::updatePaint);
        connect(element, &Element::effectPropertyUpdated, timelineContent,
                &TimelineContentWidget::updatePaint);
        bool selected = scene->selectedElements.contains(element);
        TimelineElementButton *elementButton =
            new TimelineElementButton(element, this);
        elementButtons.append(elementButton);
        timelineLeftLayout->addWidget(elementButton);

        stripe = !stripe;

        if (!element->collapsed) {
            for (auto property : element->properties) {
                addProperty(property, &stripe, elementButton, 24);
            }

            for (auto effect : element->effects) {
                if (effect->properties.empty())
                    continue;

                QPushButton *effectButton =
                    new QPushButton(timelineLeftContents);
                QString background = "transparent";
                QString backgroundSelected = "rgba(128,128,128,0.1)";
                if (stripe) {
                    background = "palette(alternate-base)";
                    backgroundSelected = "rgba(128,128,128,0.13)";
                }
                effectButton->setStyleSheet(
                    "QPushButton {"
                    "   text-align: left;"
                    "   padding-left: 32px;"
                    "   background: " +
                    background +
                    ";"
                    "   border-radius: 0px;"
                    "   font-weight: 600;"
                    "}"
                    "QPushButton[flat=\"false\"] {"
                    "   background: " +
                    backgroundSelected +
                    ";"
                    "   border-left: 3px solid palette(accent);"
                    "   padding-left: 29px;"
                    "}");
                if (effect->collapsed) {
                    effectButton->setIcon(QIcon::fromTheme("arrow-right"));
                } else {
                    effectButton->setIcon(QIcon::fromTheme("arrow-down"));
                }
                effectButton->setText(effect->effectName());
                effectButton->setFixedHeight(PROPERTY_TRACK_HEIGHT);
                effectButton->setFlat(!selected);
                connect(effectButton, &QPushButton::clicked, this,
                        [this, effect]() {
                            effect->collapsed = !effect->collapsed;
                            QTimer::singleShot(0, this,
                                               &TimelineWidget::updateContents);
                        });
                timelineLeftLayout->addWidget(effectButton);
                stripe = !stripe;

                if (effect->collapsed)
                    continue;

                for (auto property : effect->properties) {
                    addProperty(property, &stripe, elementButton, 48);
                }
            }
        }
    }

    timelineLeftLayout->addSpacing(64);
    timelineLeftLayout->addStretch();

    timelineScrolled();
}

void TimelineWidget::addProperty(PropertyBase *property, bool *stripe,
                                 QPushButton *elementButton, int indent) {
    if (!property->isAnimatable()) {
        return;
    }

    QPushButton *propertyButton = new QPushButton(timelineLeftContents);
    propertyButton->setObjectName("property");
    QString background = "transparent";
    QString backgroundSelected = "rgba(128,128,128,0.1)";
    if (*stripe) {
        background = "palette(alternate-base)";
        backgroundSelected = "rgba(128,128,128,0.13)";
    }
    propertyButton->setStyleSheet("#property {"
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
    propertyButton->setFlat(elementButton->isFlat());
    connect(propertyButton, &QPushButton::clicked, elementButton,
            &QPushButton::click);

    QHBoxLayout *propertyLayout = new QHBoxLayout(propertyButton);
    propertyLayout->setContentsMargins(indent, 0, 0, 0);
    propertyLayout->setSpacing(0);

    QPushButton *toggleAnimationButton = new QPushButton(propertyButton);

    toggleAnimationButton->setFlat(true);
    toggleAnimationButton->setFixedWidth(32);
    connect(toggleAnimationButton, &QPushButton::clicked, this,
            [this, property]() {
                property->toggleAnimating({scene->currentFrame});
            });
    propertyLayout->addWidget(toggleAnimationButton);

    QLabel *label = new QLabel(propertyButton);
    label->setText(property->getDisplayName());
    propertyLayout->addWidget(label);

    propertyLayout->addStretch();

    QPushButton *previousButton = new QPushButton(propertyButton);
    previousButton->setIcon(QIcon::fromTheme("arrow-left"));
    previousButton->setToolTip("Go to previous keyframe");
    previousButton->setFlat(true);
    previousButton->setFixedWidth(24);
    connect(previousButton, &QPushButton::clicked, this, [this, property]() {
        for (int index = property->keyframes.size() - 1; index >= 0; index--) {
            if (property->keyframes[index]->frame < scene->currentFrame) {
                scene->setFramesChanging(true);
                scene->setFrame(property->keyframes[index]->frame);
                scene->setFramesChanging(false);
                break;
            }
        }
    });
    propertyLayout->addWidget(previousButton);

    QPushButton *keyframeButton = new QPushButton(propertyButton);
    keyframeButton->setToolTip("Toggle keyframe");
    keyframeButton->setFlat(true);
    keyframeButton->setFixedWidth(24);
    connect(keyframeButton, &QPushButton::clicked, this, [this, property]() {
        if (property->has(scene->currentFrame)) {
            property->remove(scene->currentFrame);
        } else {
            property->addToPosition({scene->currentFrame});
        }
    });
    propertyLayout->addWidget(keyframeButton);

    QPushButton *nextButton = new QPushButton(propertyButton);
    nextButton->setIcon(QIcon::fromTheme("arrow-right"));
    nextButton->setToolTip("Go to next keyframe");
    nextButton->setFlat(true);
    nextButton->setFixedWidth(24);
    connect(nextButton, &QPushButton::clicked, this, [this, property]() {
        for (auto keyframe : property->keyframes) {
            if (keyframe->frame > scene->currentFrame) {
                scene->setFramesChanging(true);
                scene->setFrame(keyframe->frame);
                scene->setFramesChanging(false);
                break;
            }
        }
    });
    propertyLayout->addWidget(nextButton);

    auto updateKeyframe = [this, property, keyframeButton, previousButton,
                           nextButton]() {
        if (!property->isAnimating)
            return;

        if (property->has(scene->currentFrame)) {
            keyframeButton->setIcon(keyframeYes);
        } else {
            keyframeButton->setIcon(keyframeNo);
        }

        previousButton->setEnabled(property->hasBefore(scene->currentFrame));
        nextButton->setEnabled(property->hasAfter(scene->currentFrame));
    };

    auto propertyUpdated = [property,
                            updateKeyframe](PropertyBase *updatedProperty) {
        if (property != updatedProperty) {
            return;
        }
        updateKeyframe();
    };

    auto updateAnimating = [this, property, toggleAnimationButton,
                            previousButton, keyframeButton, nextButton,
                            updateKeyframe](PropertyBase *updatedProperty) {
        if (property != updatedProperty) {
            return;
        }

        if (property->isAnimating) {
            KIconColors colors;
            colors.setText(palette().accent().color());
            toggleAnimationButton->setIcon(KDE::icon("keyframe", colors));
            toggleAnimationButton->setToolTip("Animation enabled");
        } else {
            toggleAnimationButton->setIcon(
                QIcon::fromTheme("keyframe-disable"));
            toggleAnimationButton->setToolTip("Animation disabled");
        }

        previousButton->setVisible(property->isAnimating);
        keyframeButton->setVisible(property->isAnimating);
        nextButton->setVisible(property->isAnimating);

        updateKeyframe();

        timelineContent->updateContents();
    };

    updateAnimating(property);

    connect(property->animatable, &Animatable::propertyIsAnimatingUpdated,
            propertyButton, updateAnimating);
    connect(property->animatable, &Animatable::propertyUpdated, propertyButton,
            propertyUpdated);
    connect(scene, &Scene::frameChanged, propertyButton, updateKeyframe);

    timelineLeftLayout->addWidget(propertyButton);
    *stripe = !*stripe;
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
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);
    updateContents();
    connect(timelineWidget->scene, &Scene::frameChanged, this,
            &TimelineContentWidget::frameChanged);
}

void TimelineContentWidget::frameChanged(int frame) { update(); }

void TimelineContentWidget::updatePaint() { update(); }

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

        for (auto effect : element->effects) {
            if (effect->properties.empty())
                continue;

            height += PROPERTY_TRACK_HEIGHT;

            if (effect->collapsed)
                continue;

            for (auto property : effect->properties) {
                if (!property->isAnimatable())
                    continue;

                height += PROPERTY_TRACK_HEIGHT;
            }
        }
    }

    this->setFixedSize(secondsToPixels(timelineWidget->scene->durationFrames /
                                       timelineWidget->scene->frameRate) +
                           TIMELINE_START_OFFSET * 2,
                       height);
    update();
}

double TimelineContentWidget::secondsToPixels() {
    return timelineWidget->zoomSlider->value();
}

double TimelineContentWidget::secondsToPixels(double seconds) {
    return seconds * secondsToPixels();
}

double TimelineContentWidget::headerPos() {
    return timelineWidget->timelineMainScrollArea->verticalScrollBar()->value();
}

void TimelineContentWidget::paintProperty(QPainter &painter,
                                          PropertyBase *property, bool *stripe,
                                          int startOffset, double *yPos) {
    if (!property->isAnimatable())
        return;

    if (*stripe) {
        painter.fillRect(-startOffset, *yPos, width(), PROPERTY_TRACK_HEIGHT,
                         palette().alternateBase());
    }
    *stripe = !*stripe;

    if (property->isAnimating) {
        for (auto keyframe : property->keyframes) {
            // Diamonds
            bool isSelected = selectedKeyframes.contains(keyframe);
            bool isLinear = keyframe->easing == QEasingCurve::Linear;

            double kXPos = secondsToPixels(keyframe->frame /
                                           timelineWidget->scene->frameRate);
            double kWidth = 10;
            double kYPos = *yPos + PROPERTY_TRACK_HEIGHT / 2.0 - kWidth / 2.0;

            QPolygonF polygon;

            if (isLinear) {
                polygon << QPointF(kXPos, kYPos);
                polygon << QPointF(kXPos + kWidth / 2.0, kYPos + kWidth / 2.0);
                polygon << QPointF(kXPos, kYPos + kWidth);
                polygon << QPointF(kXPos - kWidth / 2.0, kYPos + kWidth / 2.0);
            } else {
                polygon << QPointF(kXPos - kWidth / 2.0, kYPos);
                polygon << QPointF(kXPos + kWidth / 2.0, kYPos);
                polygon << QPointF(kXPos + 1, kYPos + kWidth / 2.0);
                polygon << QPointF(kXPos + kWidth / 2.0, kYPos + kWidth);
                polygon << QPointF(kXPos - kWidth / 2.0, kYPos + kWidth);
                polygon << QPointF(kXPos - 1, kYPos + kWidth / 2.0);
            }

            if (isSelected) {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(palette().accent().color().lighter(), 3,
                                    Qt::SolidLine, Qt::SquareCap,
                                    Qt::RoundJoin));
                painter.drawPolygon(polygon);
            }
            painter.setBrush(QColor(128, 128, 128));
            painter.setPen(Qt::NoPen);
            painter.drawPolygon(polygon);

            keyframeData.append({
                .keyframe = keyframe,
                .x = kXPos + TIMELINE_START_OFFSET - kWidth / 2.0,
                .y = kYPos,
                .w = kWidth,
                .h = kWidth,
            });
        }
    }

    *yPos += PROPERTY_TRACK_HEIGHT;
}

void TimelineContentWidget::paintFrameMark(QPainter &painter, int headerPos,
                                           int frame) {
    double pixels =
        secondsToPixels((float)frame / timelineWidget->scene->frameRate);
    double nextPixel =
        secondsToPixels((float)(frame + 1) / timelineWidget->scene->frameRate);
    painter.drawRect(pixels, headerPos + TIMELINE_HEADER_HEIGHT - 2,
                     std::ceil(nextPixel - pixels) + 1, 2);
}

void TimelineContentWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    double startOffset = TIMELINE_START_OFFSET;
    painter.translate(startOffset, 0);

    double curSec =
        timelineWidget->scene->currentFrame / timelineWidget->scene->frameRate;

    double yPos = 0;
    yPos += TIMELINE_HEADER_HEIGHT;

    // --- Elements ---
    bool stripe = false;
    keyframeData.clear();
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
            paintProperty(painter, property, &stripe, startOffset, &yPos);
        }

        // --- Effects ---
        for (auto effect : element->effects) {
            if (effect->properties.empty())
                continue;

            if (stripe) {
                painter.fillRect(-startOffset, yPos, width(),
                                 PROPERTY_TRACK_HEIGHT,
                                 palette().alternateBase());
            }
            stripe = !stripe;
            yPos += PROPERTY_TRACK_HEIGHT;

            if (effect->collapsed)
                continue;

            for (auto property : effect->properties) {
                paintProperty(painter, property, &stripe, startOffset, &yPos);
            }
        }
    }

    // --- Timeline Header ---
    double headerPos = this->headerPos();
    painter.fillRect(-startOffset, headerPos, width(), TIMELINE_HEADER_HEIGHT,
                     palette().window());

    // Seconds Text
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    QFontMetrics metrics(painter.font());
    for (int second = 0; second <= timelineWidget->scene->durationFrames /
                                       timelineWidget->scene->frameRate;
         second++) {
        int x = secondsToPixels(second);

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

    painter.setRenderHint(QPainter::Antialiasing, false); // for hi-dpi
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(100, 200, 50));
    for (const auto &frame : timelineWidget->mainWindow->savedFrames) {
        paintFrameMark(painter, headerPos, frame.first);
    }

    painter.setBrush(QColor(200, 100, 0));
    for (const auto &frame : timelineWidget->mainWindow->renderingFrames) {
        paintFrameMark(painter, headerPos, frame);
    }
    painter.setRenderHint(QPainter::Antialiasing);

    // Position Marker
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().accent());
    constexpr int markerWidth = 16;
    constexpr int markerHeight = 10;
    int markerX = secondsToPixels(curSec);
    int markerY = headerPos + TIMELINE_HEADER_HEIGHT - markerHeight;
    QPolygonF polygon;
    polygon << QPointF(markerX - (markerWidth / 2.0), markerY);
    polygon << QPointF(markerX + (markerWidth / 2.0), markerY);
    polygon << QPointF(markerX + (markerWidth / 2.0),
                       markerY + markerHeight / 2.0);
    polygon << QPointF(markerX, markerY + markerHeight);
    polygon << QPointF(markerX - (markerWidth / 2.0),
                       markerY + markerHeight / 2.0);
    painter.drawPolygon(polygon);

    painter.setPen(QPen(palette().accent(), 2));
    painter.drawLine(markerX, markerY + 2, markerX, height());

    if (selecting) {
        painter.resetTransform();
        painter.setPen(palette().accent().color());
        QColor selectionFill = palette().accent().color();
        selectionFill.setAlpha(60);
        painter.setBrush(selectionFill);
        painter.drawRect(QRect(selectStart, selectEnd).normalized());
    }

    if (isMovingKeyframes && hasMoved) {
        painter.resetTransform();

        painter.setBrush(Qt::NoBrush);
        painter.setPen(palette().text().color());

        for (auto keyframe : keyframeData) {
            if (selectedKeyframes.contains(keyframe.keyframe)) {
                painter.drawText(keyframe.x, keyframe.y,
                                 QString::number(keyframe.keyframe->frame));
            }
        }
    }
}

void TimelineContentWidget::mousePressEvent(QMouseEvent *event) {
    mouseHeader = false;
    selecting = false;
    isMovingKeyframes = false;
    handleMouseRelease = true;
    hasMoved = false;
    double headerPos = this->headerPos();
    if (event->buttons() & Qt::LeftButton) {
        if (event->pos().y() < TIMELINE_HEADER_HEIGHT + headerPos) {
            mouseHeader = true;
            timelineWidget->scene->setFramesChanging(true);
            timelineWidget->scene->setFrame(
                (event->pos().x() - TIMELINE_START_OFFSET) / secondsToPixels() *
                timelineWidget->scene->frameRate);
            event->accept();
            return;
        }

        mouseClickStart = event->pos();
        selectStart = event->pos() - QPoint(1, 1);
        selectEnd = selectStart;

        KeyframeBase *clickedKeyframe{nullptr};
        for (auto keyframe : keyframeData) {
            QRect keyframeRect =
                QRect(keyframe.x, keyframe.y, keyframe.w, keyframe.h);
            if (keyframeRect.contains(selectStart)) {
                clickedKeyframe = keyframe.keyframe;
                break;
            }
        }

        if (clickedKeyframe) {
            isMovingKeyframes = true;
            selectStart = event->pos();

            if (event->modifiers().testAnyFlag(Qt::ControlModifier)) {
                if (selectedKeyframes.contains(clickedKeyframe)) {
                    selectedKeyframes.removeOne(clickedKeyframe);
                } else {
                    selectedKeyframes.append(clickedKeyframe);
                }
                handleMouseRelease = false;
            } else {
                if (!selectedKeyframes.contains(clickedKeyframe)) {
                    selectedKeyframes.clear();
                    selectedKeyframes.append(clickedKeyframe);
                }
            }

            startKeyframePositions.clear();
            for (auto keyframe : selectedKeyframes) {
                startKeyframePositions.append(keyframe->frame);
            }
        } else {
            selecting = true;
        }

        update();
    } else if (event->buttons() & Qt::RightButton) {
        KeyframeBase *hoveredKeyframe{nullptr};
        selectedKeyframes.clear();
        for (auto keyframe : keyframeData) {
            QRect keyframeRect =
                QRect(keyframe.x, keyframe.y, keyframe.w, keyframe.h);
            keyframeRect.adjust(-2, -2, 2, 2);
            if (keyframeRect.contains(event->pos())) {
                hoveredKeyframe = keyframe.keyframe;
                selectedKeyframes.append(hoveredKeyframe);
                break;
            }
        }
        update();

        if (hoveredKeyframe) {
            QMenu menu;

            menu.addSection(hoveredKeyframe->property->getDisplayName() +
                            QStringLiteral(" at frame ") +
                            QString::number(hoveredKeyframe->frame));

            QMenu *easingMenu = menu.addMenu("Easings");
            QList<QString> typeNames = {
                QStringLiteral("Linear"),

                QStringLiteral("Sine In"),        QStringLiteral("Sine Out"),
                QStringLiteral("Sine In Out"),

                QStringLiteral("Quad In"),        QStringLiteral("Quad Out"),
                QStringLiteral("Quad In Out"),

                QStringLiteral("Cubic In"),       QStringLiteral("Cubic Out"),
                QStringLiteral("Cubic In Out"),

                QStringLiteral("Quart In"),       QStringLiteral("Quart Out"),
                QStringLiteral("Quart In Out"),

                QStringLiteral("Quint In"),       QStringLiteral("Quint Out"),
                QStringLiteral("Quint In Out"),

                QStringLiteral("Expo In"),        QStringLiteral("Expo Out"),
                QStringLiteral("Expo In Out"),

                QStringLiteral("Circ In"),        QStringLiteral("Circ Out"),
                QStringLiteral("Circ In Out"),

                QStringLiteral("Back In"),        QStringLiteral("Back Out"),
                QStringLiteral("Back In Out"),

                QStringLiteral("Elastic In"),     QStringLiteral("Elastic Out"),
                QStringLiteral("Elastic In Out"),

                QStringLiteral("Bounce In"),      QStringLiteral("Bounce Out"),
                QStringLiteral("Bounce In Out"),
            };
            QList<QEasingCurve::Type> types = {
                QEasingCurve::Linear,

                QEasingCurve::InSine,       QEasingCurve::OutSine,
                QEasingCurve::InOutSine,

                QEasingCurve::InQuad,       QEasingCurve::OutQuad,
                QEasingCurve::InOutQuad,

                QEasingCurve::InCubic,      QEasingCurve::OutCubic,
                QEasingCurve::InOutCubic,

                QEasingCurve::InQuart,      QEasingCurve::OutQuart,
                QEasingCurve::InOutQuart,

                QEasingCurve::InQuint,      QEasingCurve::OutQuint,
                QEasingCurve::InOutQuint,

                QEasingCurve::InExpo,       QEasingCurve::OutExpo,
                QEasingCurve::InOutExpo,

                QEasingCurve::InCirc,       QEasingCurve::OutCirc,
                QEasingCurve::InOutCirc,

                QEasingCurve::InBack,       QEasingCurve::OutBack,
                QEasingCurve::InOutBack,

                QEasingCurve::InElastic,    QEasingCurve::OutElastic,
                QEasingCurve::InOutElastic,

                QEasingCurve::InBounce,     QEasingCurve::OutBounce,
                QEasingCurve::InOutBounce,
            };

            QActionGroup *group = new QActionGroup(easingMenu);
            for (int i = 0; i < types.size(); i++) {
                QEasingCurve::Type type = types[i];
                QAction *action = easingMenu->addAction(
                    QIcon::fromTheme(i == 0 ? "linear" : "smooth"),
                    typeNames[i]);
                action->setCheckable(true);
                action->setChecked(hoveredKeyframe->easing.type() == type);
                connect(
                    action, &QAction::triggered, action,
                    [hoveredKeyframe, type] {
                        hoveredKeyframe->easing = type;
                        hoveredKeyframe->property->animatable->_propertyUpdated(
                            hoveredKeyframe->property);
                    });

                group->addAction(action);
            }

            QAction *gotoAction = menu.addAction("Go to");
            connect(gotoAction, &QAction::triggered, this,
                    [this, hoveredKeyframe]() {
                        timelineWidget->scene->setFramesChanging(true);
                        timelineWidget->scene->setFrame(hoveredKeyframe->frame);
                        timelineWidget->scene->setFramesChanging(false);
                    });

            QAction *deleteAction =
                menu.addAction(QIcon::fromTheme("delete"), "Delete");
            connect(
                deleteAction, &QAction::triggered, this, [hoveredKeyframe]() {
                    hoveredKeyframe->property->remove(hoveredKeyframe->frame);
                });

            menu.exec(event->globalPosition().toPoint());
        }
    }
}

void TimelineContentWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        if (mouseHeader) {
            timelineWidget->scene->setFrame(
                (event->pos().x() - TIMELINE_START_OFFSET) / secondsToPixels() *
                timelineWidget->scene->frameRate);
            event->accept();
            return;
        }

        if (selecting) {
            selectEnd = event->pos() - QPoint(1, 1);
            selectedKeyframes.clear();
            QRect selectionRect = QRect(selectStart, selectEnd).normalized();
            for (auto keyframe : keyframeData) {
                QRect keyframeRect =
                    QRect(keyframe.x, keyframe.y, keyframe.w, keyframe.h);
                if (selectionRect.intersects(keyframeRect)) {
                    selectedKeyframes.append(keyframe.keyframe);
                }
            }
            update();
        }

        if (isMovingKeyframes) {
            hasMoved = true;
            selectEnd = event->pos();
            QPoint moved = selectEnd - selectStart;

            int x = moved.x();
            double seconds = x / secondsToPixels();
            int frames = seconds * timelineWidget->scene->frameRate;
            if (frames == 0)
                return;

            int index = 0;
            for (auto keyframe : selectedKeyframes) {
                if (startKeyframePositions[index] + frames < 0) {
                    frames = -startKeyframePositions[index];
                }
                index++;
            }
            index = 0;
            for (auto keyframe : selectedKeyframes) {
                keyframe->property->move(
                    keyframe->frame, startKeyframePositions[index] + frames);
                index++;
            }
            update();
        }
    }
}

void TimelineContentWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (mouseHeader) {
        timelineWidget->scene->setFramesChanging(false);
    }
    if ((selecting || isMovingKeyframes) && handleMouseRelease) {
        if ((mouseClickStart - event->pos()).isNull()) {
            selectedKeyframes.clear();
            for (auto keyframe : keyframeData) {
                QRect keyframeRect =
                    QRect(keyframe.x, keyframe.y, keyframe.w, keyframe.h);
                if (keyframeRect.contains(mouseClickStart)) {
                    selectedKeyframes.append(keyframe.keyframe);
                    break;
                }
            }
            update();
        }
    }
    if (selecting) {
        selecting = false;
        update();
    }

    if (isMovingKeyframes) {
        isMovingKeyframes = false;
        update();
    }
}

void TimelineContentWidget::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) {
        return QWidget::keyPressEvent(event);
    }

    if (event->matches(QKeySequence::Delete)) {
        deleteSelected();
        return;
    }

    return QWidget::keyPressEvent(event);
}

bool TimelineContentWidget::deleteSelected() {
    if (selectedKeyframes.isEmpty()) {
        return false;
    }

    for (auto keyframe : selectedKeyframes) {
        keyframe->property->remove(keyframe->frame);
    }
    selectedKeyframes.clear();
    update();
    return true;
}
