#include "timeline_content.hpp"
#include "animatable/element/group_element.hpp"
#include "animatable/element/text_element.hpp"
#include "gui/gui.hpp"
#include "scene.hpp"
#include "timeline.hpp"
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>

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
    this->setFixedSize(secondsToPixels(timelineWidget->scene->durationFrames /
                                       timelineWidget->scene->frameRate) +
                           TIMELINE_START_OFFSET * 2,
                       updatedHeight);
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

void TimelineContentWidget::paintElement(QPainter &painter, Element *element,
                                         double *yPos) {
    bool selected = timelineWidget->scene->selectedElements.contains(element);
    painter.setPen(Qt::NoPen);

    if (selected) {
        QLinearGradient durationGradient(0, *yPos, 0,
                                         OBJECT_TRACK_HEIGHT + *yPos);
        durationGradient.setColorAt(0, palette().accent().color().darker(110));
        durationGradient.setColorAt(1, palette().accent().color().darker(130));
        QBrush durationBrush(durationGradient);
        painter.setBrush(durationGradient);
    } else {
        painter.setBrush(palette().accent().color().darker(200));
    }
    QRectF rect{
        secondsToPixels(element->startFrame / timelineWidget->scene->frameRate),
        *yPos,
        secondsToPixels(element->durationFrames /
                        timelineWidget->scene->frameRate),
        OBJECT_TRACK_HEIGHT};
    elementRects.append(rect);
    painter.drawRect(rect);
    *yPos += OBJECT_TRACK_HEIGHT;

    if (element->collapsed)
        return;

    // --- Properties ---
    for (auto property : element->properties) {
        paintProperty(painter, property, yPos);
    }

    TextElement *textElement = dynamic_cast<TextElement *>(element);
    if (textElement) {
        for (auto animator : textElement->textAnimators) {
            *yPos += PROPERTY_TRACK_HEIGHT;

            if (animator->collapsed)
                continue;

            for (auto selector : animator->selectors) {
                *yPos += PROPERTY_TRACK_HEIGHT;

                if (selector->collapsed)
                    continue;

                for (auto property : selector->properties) {
                    paintProperty(painter, property, yPos);
                }
            }

            for (auto property : animator->properties) {
                paintProperty(painter, property, yPos);
            }
        }
    }

    // --- Effects ---
    for (auto effect : element->effects) {
        if (effect->properties.empty())
            continue;

        *yPos += PROPERTY_TRACK_HEIGHT;

        if (effect->collapsed)
            continue;

        for (auto property : effect->properties) {
            paintProperty(painter, property, yPos);
        }
    }

    GroupElement *groupElement = dynamic_cast<GroupElement *>(element);

    if (groupElement) {
        for (auto child : groupElement->getChildren()) {
            paintElement(painter, child, yPos);
        }
    }
}

void TimelineContentWidget::paintProperty(QPainter &painter,
                                          PropertyBase *property,
                                          double *yPos) {
    if (property->hidden || !property->isAnimatable())
        return;

    painter.fillRect(-TIMELINE_START_OFFSET, *yPos, width(), 1,
                     palette().mid());
    painter.fillRect(-TIMELINE_START_OFFSET, *yPos + PROPERTY_TRACK_HEIGHT,
                     width(), 1, palette().mid());

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
    keyframeData.clear();
    elementRects.clear();

    for (auto element : timelineWidget->scene->elements) {
        if (element->hasParent())
            continue;

        paintElement(painter, element, &yPos);
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
    isResizingElements = false;
    isResizingOut = false;
    double headerPos = this->headerPos();
    if (event->buttons() & Qt::LeftButton) {
        if (event->pos().y() < TIMELINE_HEADER_HEIGHT + headerPos) {
            mouseHeader = true;
            timelineWidget->scene->setFramesChanging(true);
            timelineWidget->scene->setFrame(std::round(
                (event->pos().x() - TIMELINE_START_OFFSET) / secondsToPixels() *
                timelineWidget->scene->frameRate));
            event->accept();
            return;
        }

        mouseClickStart = event->pos();

        QPointF shiftedPos =
            event->position() - QPointF{TIMELINE_START_OFFSET, 0};
        for (int i = 0; i < elementRects.length(); i++) {
            const auto &elementRect = elementRects[i];
            if (QRectF(elementRect.x() - 12, elementRect.y(), 24,
                       elementRect.height())
                    .contains(shiftedPos)) {
                isResizingElements = true;
                isResizingOut = false;
            }
            if (QRectF(elementRect.x() + elementRect.width() - 12,
                       elementRect.y(), 24, elementRect.height())
                    .contains(shiftedPos)) {
                isResizingElements = true;
                isResizingOut = true;
            }

            if (elementRect.contains(shiftedPos) || isResizingElements) {
                Element *element = timelineWidget->scene->elements[i];
                if (!timelineWidget->scene->selectedElements.contains(
                        element)) {
                    timelineWidget->scene->selectElements({element});
                }
                startMousePosition = event->position().x();
                startElementMove.clear();
                for (auto selected : timelineWidget->scene->selectedElements) {
                    startElementMove.append(selected->startFrame);
                }
                if (isResizingElements) {
                    startElementSizes.clear();
                    for (auto selected :
                         timelineWidget->scene->selectedElements) {
                        startElementSizes.append(selected->durationFrames);
                    }
                }
                return;
            }
        }

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
    setCursor(Qt::CursorShape::ArrowCursor);
    QPointF shiftedPos = event->position() - QPointF{TIMELINE_START_OFFSET, 0};
    for (const auto &elementRect : elementRects) {
        if (QRectF(elementRect.x() - 12, elementRect.y(), 24,
                   elementRect.height())
                .contains(shiftedPos) ||
            QRectF(elementRect.x() + elementRect.width() - 12, elementRect.y(),
                   24, elementRect.height())
                .contains(shiftedPos)) {
            setCursor(Qt::CursorShape::SizeHorCursor);
        }
    }
    if (event->buttons() & Qt::LeftButton) {
        if (mouseHeader) {
            timelineWidget->scene->setFrame(std::round(
                (event->pos().x() - TIMELINE_START_OFFSET) / secondsToPixels() *
                timelineWidget->scene->frameRate));
            event->accept();
            return;
        }

        if (!startElementMove.isEmpty()) {
            double offset = event->position().x() - startMousePosition;
            int framesMoved =
                std::round(offset / secondsToPixels(
                                        1. / timelineWidget->scene->frameRate));
            bool isResizing = !startElementSizes.isEmpty();
            for (int i = 0;
                 i < timelineWidget->scene->selectedElements.length(); i++) {
                Element *element = timelineWidget->scene->selectedElements[i];
                if (isResizing && isResizingOut) {
                    element->durationFrames =
                        std::clamp(startElementSizes[i] + framesMoved, 1,
                                   timelineWidget->scene->durationFrames -
                                       element->startFrame);
                } else if (isResizing && !isResizingOut) {
                    element->durationFrames =
                        std::clamp(startElementSizes[i] - framesMoved, 1,
                                   startElementMove[i] + startElementSizes[i]);
                    element->startFrame = startElementSizes[i] -
                                          element->durationFrames +
                                          startElementMove[i];
                } else {
                    element->startFrame =
                        std::clamp(startElementMove[i] + framesMoved, 0,
                                   timelineWidget->scene->durationFrames -
                                       element->durationFrames);
                }
            }
            timelineWidget->mainWindow
                ->invalidateAndRerender(); // TODO: should use signals
            update();
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

            int closestDiff = 8;
            int newFrame = INT32_MIN;

            if (!QApplication::queryKeyboardModifiers().testFlag(
                    Qt::ControlModifier)) {
                QList<int> snapFrames;
                snapFrames.append(timelineWidget->scene->currentFrame);
                for (const auto &keyframeOne : keyframeData) {
                    if (selectedKeyframes.contains(keyframeOne.keyframe))
                        continue;
                    snapFrames.append(keyframeOne.keyframe->frame);
                }

                for (auto pos : startKeyframePositions) {
                    int target = pos + frames;
                    for (auto snapFrame : snapFrames) {
                        int diff = qAbs(snapFrame - target);
                        if (diff < closestDiff) {
                            newFrame = snapFrame - pos;
                        }
                    }
                }
            }

            if (newFrame != INT32_MIN) {
                frames = newFrame;
            }

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
    if (!startElementMove.isEmpty()) {
        startElementMove.clear();
    }
    if (isResizingElements) {
        startElementSizes.clear();
        isResizingElements = false;
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
