#include "timeline.hpp"
#include "animatable/element/group_element.hpp"
#include "animatable/element/text_element.hpp"
#include "element_button.hpp"
#include "gui/line.hpp"
#include "gui/push_button.hpp"
#include "property_button.hpp"
#include "timeline_content.hpp"
#include <QActionGroup>
#include <QApplication>
#include <QDragEnterEvent>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QSplitter>

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
            &TimelineWidget::elementSelectionChanged);
    connect(scene, &Scene::elementOrderChanged, this,
            &TimelineWidget::updateContents);
    connect(scene, &Scene::sceneInfoChanged, this,
            &TimelineWidget::updateContents);
    connect(scene, &Scene::framesChanging, this,
            &TimelineWidget::framesChanging); // hack for property not updating
                                              // in inline PropertyEdit

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

    toolbarLay->addSpacing(16);
    positionInput = new QDoubleSpinBox(toolbar);
    positionInput->setRange(0, INT32_MAX);
    positionInput->setToolTip("Current position");
    connect(positionInput, &QDoubleSpinBox::valueChanged, this,
            &TimelineWidget::positionChanged);
    toolbarLay->addWidget(positionInput);

    positionTypeMenu = new QMenu(this);

    QActionGroup *group = new QActionGroup(positionTypeMenu);
    group->setExclusive(true);

    actionSeconds = group->addAction("Seconds");
    actionSeconds->setCheckable(true);
    actionSeconds->setChecked(true);
    positionTypeMenu->addAction(actionSeconds);
    actionFrames = group->addAction("Frames");
    actionFrames->setCheckable(true);
    positionTypeMenu->addAction(actionFrames);

    positionTypeButton = new QPushButton(toolbar);
    positionTypeButton->setIcon(QIcon::fromTheme("arrow-down"));
    connect(positionTypeButton, &QPushButton::pressed, this,
            &TimelineWidget::showPositionTypeMenu);
    toolbarLay->addWidget(positionTypeButton);

    toolbarLay->addStretch();

    zoomSlider = new QSlider(toolbar);
    zoomSlider->setOrientation(Qt::Horizontal);
    zoomSlider->setRange(1, 250);
    zoomSlider->setValue(30);
    zoomSlider->setMaximumWidth(150);
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

    splitter->setSizes({300, 900});

    lay->addWidget(splitter);

    timelineLeftContents->installEventFilter(this);
    timelineMainScrollArea->viewport()->installEventFilter(this);

    connect(zoomSlider, &QSlider::valueChanged, timelineContent,
            &TimelineContentWidget::updateContents);

    connect(scene, &Scene::frameChanged, this, &TimelineWidget::frameChanged);
    connect(scene, &Scene::sceneInfoChanged, this,
            &TimelineWidget::sceneInfoChanged);
    frameChanged(0);
    sceneInfoChanged();

    updateContents();
}

void TimelineWidget::showPositionTypeMenu() {
    positionTypeMenu->exec(positionTypeButton->mapToGlobal(
        positionTypeButton->rect().bottomLeft()));
    positionTypeButton->clearFocus();
    sceneInfoChanged();
}

void TimelineWidget::frameChanged(int frame) {
    QSignalBlocker blocker(positionInput);
    if (actionFrames->isChecked()) {
        positionInput->setValue(frame);
    } else {
        positionInput->setValue((double)frame / scene->frameRate);
    }
}

void TimelineWidget::sceneInfoChanged() {
    QSignalBlocker blocker(positionInput);
    if (actionFrames->isChecked()) {
        positionInput->setRange(0, scene->durationFrames - 1);
        positionInput->setDecimals(0);
        positionInput->setSuffix("");
    } else {
        positionInput->setRange(0,
                                (scene->durationFrames - 1) / scene->frameRate);
        positionInput->setDecimals(2);
        positionInput->setSuffix(" s");
    }
    frameChanged(scene->currentFrame);
}

void TimelineWidget::positionChanged(double value) {
    scene->setFramesChanging(true);
    if (actionFrames->isChecked()) {
        scene->setFrame(positionInput->value());
    } else {
        scene->setFrame(positionInput->value() * scene->frameRate);
    }
    scene->setFramesChanging(false);
}

void TimelineWidget::framesChanging(bool changing) {
    if (!changing) {
        updateContents();
    }
}

void TimelineWidget::elementSelectionChanged(QList<Element *> elements) {
    timelineContent->selectedKeyframes.clear();
    updateContents();
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
        // TODO: IMPORTANT: reorder for grouped elements
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
    if (obj == timelineMainScrollArea->viewport()) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = (QWheelEvent *)event;
            if (wheelEvent->modifiers().testFlag(
                    Qt::KeyboardModifier::ControlModifier)) {
                float scale = wheelEvent->angleDelta().y() / 120.f;
                int zoomValue = zoomSlider->value();
                zoomSlider->setValue(
                    std::ceil(zoomValue * std::pow(1.1, scale)));
                return true;
            }
        }
    }

    return QObject::eventFilter(obj, event);
}

void TimelineWidget::addElement(Element *element, int indent) {
    connect(element, &Element::effectListUpdated, this,
            &TimelineWidget::updateContents);
    connect(element, &Element::propertyUpdated, timelineContent,
            &TimelineContentWidget::updatePaint);
    connect(element, &Element::propertyIsAnimatingUpdated, timelineContent,
            &TimelineContentWidget::updatePaint);
    bool selected = scene->selectedElements.contains(element);
    TimelineElementButton *elementButton =
        new TimelineElementButton(element, this, indent);
    elementButtons.append(elementButton);
    timelineLeftLayout->addWidget(elementButton);
    timelineContent->updatedHeight += OBJECT_TRACK_HEIGHT;

    if (!element->collapsed) {
        for (auto property : element->properties) {
            addProperty(property, elementButton, 24 + indent, false);
        }

        TextElement *textElement = dynamic_cast<TextElement *>(element);
        if (textElement) {
            for (auto animator : textElement->textAnimators) {
                if (!addCollapsible(animator, selected, 32 + indent))
                    continue;

                for (auto selector : animator->selectors) {
                    if (!addCollapsible(selector, selected, 48 + indent))
                        continue;

                    for (auto selector : selector->properties) {
                        addProperty(selector, elementButton, 64 + indent, true);
                    }
                }

                for (auto property : animator->properties) {
                    addProperty(property, elementButton, 48 + indent, true);
                }
            }
        }

        for (auto effect : element->effects) {
            if (effect->properties.empty())
                continue;

            if (!addCollapsible(effect, selected, 32 + indent))
                continue;

            for (auto property : effect->properties) {
                addProperty(property, elementButton, 48 + indent, false);
            }
        }

        GroupElement *groupElement = dynamic_cast<GroupElement *>(element);
        if (groupElement) {
            for (auto child : groupElement->getChildren()) {
                addElement(child, indent + 18);
            }
        }
    }
}

void TimelineWidget::updateContents() {
    freezeTimelineScroll = true;
    while (timelineLeftLayout->count() > 0) {
        auto item = timelineLeftLayout->takeAt(0);
        QWidget *widget = item->widget();
        if (widget != nullptr) {
            widget->hide();
            widget->setParent(nullptr);
            widget->blockSignals(true);
            widget->deleteLater();
        }
        delete item;
    }

    timelineLeftLayout->addSpacing(TIMELINE_HEADER_HEIGHT);

    elementButtons.clear();
    timelineContent->updatedHeight = TIMELINE_HEADER_HEIGHT;

    for (auto element : scene->elements) {
        disconnect(element, &Element::effectListUpdated, this,
                   &TimelineWidget::updateContents);
        disconnect(element, &Element::propertyUpdated, timelineContent,
                   &TimelineContentWidget::updatePaint);
        disconnect(element, &Element::propertyIsAnimatingUpdated,
                   timelineContent, &TimelineContentWidget::updatePaint);
    }

    for (auto element : scene->elements) {
        if (element->hasParent())
            continue;

        addElement(element, 0);
    }

    timelineLeftLayout->addSpacing(64);
    timelineLeftLayout->addStretch();
    freezeTimelineScroll = false;
    timelineScrolled();
}

bool TimelineWidget::addCollapsible(ICollapsible *collapsible, bool selected,
                                    int indent) {
    bool collapsed = collapsible->isCollapsed();

    timelineContent->updatedHeight += PROPERTY_TRACK_HEIGHT;
    PushButton *effectButton = new PushButton(timelineLeftContents);
    QString background = "transparent";
    QString backgroundSelected = "rgba(128,128,128,0.1)";
    effectButton->setStyleSheet("QPushButton {"
                                "   text-align: left;"
                                "   padding-left: " +
                                QString::number(indent) +
                                "px;"
                                "   background: " +
                                background +
                                ";"
                                "   border-radius: 0px;"
                                "   font-weight: 600;"
                                "   border-top: 1px solid palette(midlight);"
                                "}"
                                "QPushButton[flat=\"false\"] {"
                                "   background: " +
                                backgroundSelected +
                                ";"
                                "   border-left: 3px solid palette(accent);"
                                "   padding-left: " +
                                QString::number(indent - 3) +
                                "px;"
                                "}");
    if (collapsed) {
        effectButton->setIcon(QIcon::fromTheme("arrow-right"));
    } else {
        effectButton->setIcon(QIcon::fromTheme("arrow-down"));
    }
    effectButton->setText(collapsible->displayName());
    effectButton->setFixedHeight(PROPERTY_TRACK_HEIGHT);
    effectButton->setFlat(!selected);
    connect(effectButton, &QPushButton::clicked, this, [this, collapsible]() {
        collapsible->setCollapsed(!collapsible->isCollapsed());
        QTimer::singleShot(0, this, &TimelineWidget::updateContents);
    });

    connect(effectButton, &PushButton::rightClicked, this,
            [collapsible, effectButton]() {
                QMenu menu;
                QAction *collapseAction;
                if (collapsible->isCollapsed()) {
                    collapseAction = menu.addAction("Expand");
                } else {
                    collapseAction = menu.addAction("Collapse");
                }
                QAction *deleteAction = nullptr;
                if (collapsible->isDeletable()) {
                    deleteAction = menu.addAction("Delete");
                    deleteAction->setIcon(QIcon::fromTheme("delete"));
                }
                QAction *action = menu.exec(QCursor::pos());

                if (action == collapseAction) {
                    effectButton->click();
                } else if (action == deleteAction) {
                    collapsible->deleteThis();
                }
            });

    timelineLeftLayout->addWidget(effectButton);

    return !collapsed;
}

void TimelineWidget::addProperty(PropertyBase *property,
                                 QPushButton *elementButton, int indent,
                                 bool showEdit) {
    if (property->hidden || !property->isAnimatable()) {
        return;
    }
    timelineContent->updatedHeight += PROPERTY_TRACK_HEIGHT;

    TimelinePropertyButton *propertyButton = new TimelinePropertyButton(
        property, scene, this, elementButton, indent, showEdit);

    timelineLeftLayout->addWidget(propertyButton);
}

void TimelineWidget::timelineScrolled() {
    if (freezeTimelineScroll)
        return;

    timelineLeftScrollArea->verticalScrollBar()->setValue(
        timelineMainScrollArea->verticalScrollBar()->value());

    timelineContent->updateContents();
}
