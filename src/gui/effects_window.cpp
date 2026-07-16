#include "effects_window.hpp"
#include "animatable/effect/blur_effect.hpp"
#include "animatable/effect/grid_effect.hpp"
#include "animatable/effect/invert_effect.hpp"
#include "animatable/effect/scale_effect.hpp"
#include "animatable/effect/wave_effect.hpp"
#include "line.hpp"
#include "property_edit.hpp"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

EffectsWindow::EffectsWindow(Scene *scene) : scene(scene) {
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
        QString name = effect->effectName();
        QPushButton *effectButton = new QPushButton(scrollContents);
        effectButton->setFlat(true);
        effectButton->setObjectName("effectButton");
        effectButton->setStyleSheet(
            "#effectButton {border-radius: 0px; border-top: 1px solid "
            "palette(light); border-bottom: 1px solid palette(light); "
            "background: palette(mid);}");

        QHBoxLayout *effectButtonLayout = new QHBoxLayout(effectButton);
        effectButtonLayout->setContentsMargins(8, 0, 0, 0);

        QLabel *lbl = new QLabel(name, effectButton);
        effectButtonLayout->addWidget(lbl);
        effectButtonLayout->addStretch();

        QPushButton *deleteButton = new QPushButton(effectButton);
        deleteButton->setFlat(true);
        deleteButton->setIcon(QIcon::fromTheme("window-close"));
        connect(deleteButton, &QPushButton::clicked, this,
                [element, effect]() { element->removeEffect(effect); });
        effectButtonLayout->addWidget(deleteButton);

        effectsLayout->addWidget(effectButton);

        QWidget *propertiesWidget = new QWidget(scrollContents);
        QFormLayout *propertiesLayout = new QFormLayout(propertiesWidget);
        for (auto property : effect->properties) {
            PropertyEdit *propertyEdit =
                new PropertyEdit(property, scene, propertiesWidget);
            propertiesLayout->addRow(property->getDisplayName(), propertyEdit);
        }
        effectsLayout->addWidget(propertiesWidget);
    }

    stackedWidget->setCurrentIndex(1);

    effectUpdateConnection = connect(element, &Element::effectListUpdated, this,
                                     &EffectsWindow::effectListUpdated);
}

QMenu *EffectsWindow::createEffectsMenu(QWidget *parent) {
    QMenu *menu = new QMenu(parent);

    QAction *action;
    QMenu *blurs = menu->addMenu("Blur");
    action = blurs->addAction("Box Blur");
    action->setData("blur");
    QMenu *tools = menu->addMenu("Tools");
    action = tools->addAction("Invert");
    action->setData("invert");
    action = tools->addAction("Scale");
    action->setData("scale");
    QMenu *style = menu->addMenu("Style");
    action = style->addAction("Wave");
    action->setData("wave");
    QMenu *generate = menu->addMenu("Generate");
    action = generate->addAction("Grid");
    action->setData("grid");

    connect(menu, &QMenu::triggered, this, &EffectsWindow::addEffectTriggered);

    return menu;
}

void EffectsWindow::addEffectTriggered(QAction *action) {
    QString effectType = action->data().toString();
    Effect *effect{nullptr};

    if (effectType == "invert") {
        effect = new InvertEffect();
    } else if (effectType == "blur") {
        effect = new BlurEffect();
    } else if (effectType == "wave") {
        effect = new WaveEffect();
    } else if (effectType == "grid") {
        effect = new GridEffect();
    } else if (effectType == "scale") {
        effect = new ScaleEffect();
    }

    if (effect) {
        scene->selectedElements[0]->addEffect(effect);
    }
}
