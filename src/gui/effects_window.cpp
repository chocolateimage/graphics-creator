#include "effects_window.hpp"
#include "animatable/effect/effect_list.hpp"
#include "line.hpp"
#include "property_edit.hpp"
#include <QApplication>
#include <QDrag>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

const QString EFFECT_DRAG_MIME_TYPE =
    "application/x-graphicscreator-effect-drag";

EffectWidget::EffectWidget(Scene *scene, Element *element, Effect *effect,
                           QWidget *parent)
    : QWidget(parent), scene(scene), element(element), effect(effect) {
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);
    QString name = effect->effectName();
    QString displayName = name;
    for (const auto &effect : effectList) {
        if (effect.name == name) {
            displayName = effect.displayName;
            break;
        }
    }
    effectButton = new QPushButton(this);
    effectButton->setFlat(true);
    effectButton->setObjectName("effectButton");
    effectButton->setStyleSheet(
        "#effectButton {border-radius: 0px; border-top: 1px solid "
        "palette(light); border-bottom: 1px solid palette(light); "
        "background: palette(mid);}");
    effectButton->installEventFilter(this);

    QHBoxLayout *effectButtonLayout = new QHBoxLayout(effectButton);
    effectButtonLayout->setContentsMargins(8, 0, 0, 0);
    effectButtonLayout->setSpacing(0);

    collapseButton = new QPushButton(effectButton);
    collapseButton->setFlat(true);
    collapseButton->setIcon(effect->collapsed ? QIcon::fromTheme("arrow-right")
                                              : QIcon::fromTheme("arrow-down"));
    connect(collapseButton, &QPushButton::clicked, this,
            &EffectWidget::collapseClick);
    effectButtonLayout->addWidget(collapseButton);

    QLabel *lbl = new QLabel(displayName, effectButton);
    effectButtonLayout->addWidget(lbl);
    effectButtonLayout->addStretch();

    QPushButton *deleteButton = new QPushButton(effectButton);
    deleteButton->setFlat(true);
    deleteButton->setIcon(QIcon::fromTheme("window-close"));
    connect(deleteButton, &QPushButton::clicked, this,
            &EffectWidget::deleteClick);
    effectButtonLayout->addWidget(deleteButton);

    lay->addWidget(effectButton);

    propertiesWidget = new QWidget(this);
    propertiesWidget->setVisible(!effect->collapsed);
    QFormLayout *propertiesLayout = new QFormLayout(propertiesWidget);
    for (auto property : effect->properties) {
        PropertyEdit *propertyEdit =
            new PropertyEdit(property, scene, propertiesWidget);
        propertiesLayout->addRow(property->getDisplayName(), propertyEdit);
    }
    connect(effect, &Effect::collapsedChanged, this,
            &EffectWidget::collapsedChanged);
    lay->addWidget(propertiesWidget);
}

void EffectWidget::deleteClick() { element->removeEffect(effect); }

void EffectWidget::collapseClick() { effect->setCollapsed(!effect->collapsed); }

void EffectWidget::collapsedChanged() {
    collapseButton->setIcon(effect->collapsed ? QIcon::fromTheme("arrow-right")
                                              : QIcon::fromTheme("arrow-down"));
    propertiesWidget->setVisible(!effect->collapsed);
}

bool EffectWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == effectButton) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragStartPosition = mouseEvent->pos();
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                if ((mouseEvent->pos() - dragStartPosition).manhattanLength() >=
                    QApplication::startDragDistance()) {

                    QDrag *drag = new QDrag(this);
                    QMimeData *mimeData = new QMimeData();
                    mimeData->setData(
                        EFFECT_DRAG_MIME_TYPE,
                        QString::number((uint64_t)(effect)).toUtf8());
                    drag->setMimeData(mimeData);

                    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
                }
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

EffectsWindow::EffectsWindow(Scene *scene) : scene(scene) {
    setAcceptDrops(true);

    stackedWidget = new QStackedWidget(this);
    topMainLayout = new QVBoxLayout(this);
    topMainLayout->setContentsMargins(0, 0, 0, 0);
    topMainLayout->addWidget(stackedWidget);

    errorLabel = new QLabel();
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setDisabled(true);
    errorLabel->setWordWrap(true);
    stackedWidget->addWidget(errorLabel);

    mainWidget = new QWidget();
    stackedWidget->addWidget(mainWidget);
    mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QFrame *topFrame = new QFrame(mainWidget);
    topFrame->setBackgroundRole(QPalette::Mid);
    topFrame->setAutoFillBackground(true);
    QHBoxLayout *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(8, 0, 8, 0);
    effectCountLabel = new QLabel(topFrame);
    effectCountLabel->setDisabled(true);
    effectCountLabel->setWordWrap(true);
    topLayout->addWidget(effectCountLabel);
    topLayout->addStretch();
    QPushButton *addButton = new QPushButton("Add effect", topFrame);
    addButton->setMenu(createEffectsMenu(addButton));
    addButton->menu()->setMinimumWidth(addButton->width());
    addButton->setFlat(true);
    topLayout->addWidget(addButton);
    mainLayout->addWidget(topFrame);

    mainLayout->addWidget(new HorizontalLine(mainWidget));

    scrollArea = new QScrollArea(mainWidget);
    scrollArea->setWidgetResizable(true);
    scrollContents = new QWidget(scrollArea);
    scrollArea->setWidget(scrollContents);

    scrollArea->show();
    scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    mainLayout->addWidget(scrollArea, 1);

    effectsLayout = new QVBoxLayout(scrollContents);
    effectsLayout->setAlignment(Qt::AlignTop);
    effectsLayout->setContentsMargins(0, 0, 0, 0);
    effectsLayout->setSpacing(0);

    connect(scene, &Scene::elementSelectionChanged, this,
            &EffectsWindow::selectedElementsUpdated);
    connect(scene, &Scene::framesChanging, this,
            &EffectsWindow::framesChanging);
    selectedElementsUpdated({});
}

void EffectsWindow::framesChanging(bool changing) {
    if (changing) {
        setDisabled(true);
        return;
    }

    setDisabled(false);
    selectedElementsUpdated(scene->selectedElements);
}

void EffectsWindow::effectListUpdated() {
    selectedElementsUpdated(scene->selectedElements);
}

void EffectsWindow::selectedElementsUpdated(QList<Element *> selectedElements) {
    disconnect(effectUpdateConnection);
    effectWidgets.clear();

    while (effectsLayout->count() > 0) {
        auto item = effectsLayout->takeAt(0);
        QWidget *widget = item->widget();
        if (widget != nullptr) {
            widget->hide();
            widget->setParent(nullptr);
            widget->deleteLater();
        }
        delete item;
    }

    if (selectedElements.size() == 0) {
        errorLabel->setText("No elements selected");
        stackedWidget->setCurrentIndex(0);
        return;
    }

    if (selectedElements.size() > 1) {
        errorLabel->setText(
            "Multiple elements cannot be edited at the same time");
        stackedWidget->setCurrentIndex(0);
        return;
    }

    Element *element = selectedElements.first();

    int effectCount = element->effects.size();
    if (effectCount == 1) {
        effectCountLabel->setText(QString::number(effectCount) + " effect");
    } else {
        effectCountLabel->setText(QString::number(effectCount) + " effects");
    }

    for (Effect *effect : element->effects) {
        EffectWidget *effectWidget =
            new EffectWidget(scene, element, effect, scrollContents);
        effectWidgets.append(effectWidget);
        effectsLayout->addWidget(effectWidget);
    }

    stackedWidget->setCurrentIndex(1);

    effectUpdateConnection = connect(element, &Element::effectListUpdated, this,
                                     &EffectsWindow::effectListUpdated);
}

QMenu *EffectsWindow::createEffectsMenu(QWidget *parent) {
    QMenu *menu = new QMenu(parent);

    QAction *action;
    QMap<QString, QMenu *> menus;

    for (const auto &effectInfo : effectList) {
        QMenu *categoryMenu;
        if (menus.contains(effectInfo.category)) {
            categoryMenu = menus[effectInfo.category];
        } else {
            categoryMenu = menu->addMenu(effectInfo.category);
            menus[effectInfo.category] = categoryMenu;
        }
        QAction *action = categoryMenu->addAction(effectInfo.displayName);
        action->setData(effectInfo.name);
    }

    connect(menu, &QMenu::triggered, this, &EffectsWindow::addEffectTriggered);

    return menu;
}

void EffectsWindow::addEffectTriggered(QAction *action) {
    QString effectType = action->data().toString();
    Effect *effect{nullptr};

    for (const auto &effectInfo : effectList) {
        if (effectInfo.name == effectType) {
            effect = effectInfo.create();
            break;
        }
    }

    if (effect) {
        scene->selectedElements[0]->addEffect(effect);
    }
}

void EffectsWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat(EFFECT_DRAG_MIME_TYPE)) {
        event->accept();
    }
}

void EffectsWindow::dragMoveEvent(QDragMoveEvent *event) {
    if (event->mimeData()->hasFormat(EFFECT_DRAG_MIME_TYPE)) {
        if (!effectMoveBar) {
            effectMoveBar = new QFrame(this);
            effectMoveBar->setFrameShape(QFrame::Shape::StyledPanel);
            effectMoveBar->setFrameShadow(QFrame::Shadow::Raised);
            effectMoveBar->setFixedWidth(width());
            effectMoveBar->setFixedHeight(2);
            effectMoveBar->setAutoFillBackground(true);
            effectMoveBar->setBackgroundRole(QPalette::ColorRole::Accent);
            effectMoveBar->move(0, -100);
            effectMoveBar->show();
        }

        QList<int> yPositions;

        int offset = scrollArea->y() - scrollArea->verticalScrollBar()->value();
        yPositions.append(effectWidgets[0]->y() + offset);

        for (auto widget : effectWidgets) {
            yPositions.append(widget->y() + widget->height() + offset);
        }

        QPointF pos = mapFromGlobal(mapToGlobal(event->position()));
        int yPos = pos.y();
        int targetEffectIndex = 0;
        int lastClosest = INT32_MAX;
        for (int i = 0; i < yPositions.size(); i++) {
            int diff = std::abs(yPositions[i] - yPos);
            if (diff < lastClosest) {
                targetEffectIndex = i;
                lastClosest = diff;
            }
        }
        effectMoveTarget = targetEffectIndex;
        effectMoveBar->move(0, yPositions[targetEffectIndex]);
    }
}

void EffectsWindow::dragLeaveEvent(QDragLeaveEvent *event) {
    if (effectMoveBar) {
        effectMoveBar->deleteLater();
        effectMoveBar = nullptr;
    }
}

void EffectsWindow::dropEvent(QDropEvent *event) {
    if (effectMoveBar) {
        uint64_t address =
            QString::fromUtf8(event->mimeData()->data(EFFECT_DRAG_MIME_TYPE))
                .toULongLong();
        Effect *gotEffect = (Effect *)address;
        scene->selectedElements[0]->reorderEffect(gotEffect, effectMoveTarget);

        effectMoveBar->deleteLater();
        effectMoveBar = nullptr;
    }
}
