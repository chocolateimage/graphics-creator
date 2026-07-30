#include "element_button.hpp"
#include "timeline.hpp"
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

TimelineElementButton::TimelineElementButton(Element *element,
                                             TimelineWidget *timelineWidget)
    : QPushButton(timelineWidget->timelineLeftContents),
      timelineWidget(timelineWidget), element(element) {
    bool selected = timelineWidget->scene->selectedElements.contains(element);
    setObjectName("timelineElementButton");
    setStyleSheet("#timelineElementButton {"
                  "   text-align: left;"
                  "   padding-left: 8px;"
                  "   background: transparent;"
                  "   border-radius: 0px;"
                  "   font-weight: 600;"
                  "}"
                  "#timelineElementButton:hover {"
                  "   background: rgba(128,128,128,0.1);"
                  "}"
                  "#timelineElementButton[flat=\"false\"] {"
                  "   background: rgba(128,128,128,0.25);"
                  "   border-left: 3px solid palette(accent);"
                  "   padding-left: 5px;"
                  "}"
                  "#timelineElementButton:pressed {"
                  "   background: rgba(128,128,128,0.3);"
                  "}");
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(8, 0, 8, 0);
    lay->setSpacing(0);

    QPushButton *collapseButton = new QPushButton(this);
    collapseButton->setFlat(true);
    collapseButton->setFixedWidth(24);
    if (element->collapsed) {
        collapseButton->setIcon(QIcon::fromTheme("arrow-right"));
        collapseButton->setToolTip("Expand");
    } else {
        collapseButton->setIcon(QIcon::fromTheme("arrow-down"));
        collapseButton->setToolTip("Collapse");
    }
    connect(collapseButton, &QPushButton::clicked, this,
            &TimelineElementButton::collapseClicked);
    lay->addWidget(collapseButton);

    objectNameLabel = new QLabel(this);
    objectNameLabel->setText(element->objectName());
    lay->addWidget(objectNameLabel);

    lay->addStretch();

    visibilityButton = new QPushButton(this);
    visibilityButton->setFlat(true);
    visibilityButton->setFixedWidth(24);
    visibilityUpdated();
    connect(visibilityButton, &QPushButton::clicked, this,
            &TimelineElementButton::visibilityClicked);
    lay->addWidget(visibilityButton);

    setFixedHeight(OBJECT_TRACK_HEIGHT);
    setFlat(!selected);

    connect(element, &Element::objectNameChanged, this,
            &TimelineElementButton::elementNameChanged);
    connect(element, &Element::visibilityUpdated, this,
            &TimelineElementButton::visibilityUpdated);
    connect(this, &QPushButton::clicked, this,
            &TimelineElementButton::clickedSlot);
}

void TimelineElementButton::visibilityClicked() {
    element->setVisible(!element->visible);
}

void TimelineElementButton::visibilityUpdated() {
    if (element->visible) {
        visibilityButton->setToolTip("Hide");
        visibilityButton->setIcon(QIcon::fromTheme("view-visible"));
    } else {
        visibilityButton->setToolTip("Show");
        visibilityButton->setIcon(QIcon::fromTheme("view-visible-off"));
    }
}

void TimelineElementButton::elementNameChanged(const QString &objectName) {
    objectNameLabel->setText(objectName);
}

void TimelineElementButton::collapseClicked() {
    element->collapsed = !element->collapsed;
    QTimer::singleShot(0, timelineWidget, &TimelineWidget::updateContents);
}

void TimelineElementButton::clickedSlot() {
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
