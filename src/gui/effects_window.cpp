#include "effects_window.hpp"
#include "animatable/effect/blur_effect.hpp"
#include "animatable/effect/invert_effect.hpp"
#include "line.hpp"
#include "property_edit.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

EffectsWindow::EffectsWindow(Scene *scene) : scene(scene) {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
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

    while (mainLayout->count() > 0) {
        auto item = mainLayout->takeAt(0);
        QWidget *widget = item->widget();
        if (widget != nullptr) {
            widget->hide();
            widget->setParent(nullptr);
            widget->deleteLater();
        }
        delete item;
    }

    if (selectedElements.size() == 0) {
        QLabel *lbl = new QLabel("No elements selected", this);
        lbl->setDisabled(true);
        lbl->setWordWrap(true);
        mainLayout->addWidget(lbl);
        return;
    }

    if (selectedElements.size() > 1) {
        QLabel *lbl = new QLabel(
            "Multiple elements cannot be edited at the same time", this);
        lbl->setDisabled(true);
        lbl->setWordWrap(true);
        mainLayout->addWidget(lbl);
        return;
    }

    Element *element = selectedElements.first();
    QFrame *topFrame = new QFrame(this);
    QHBoxLayout *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(8, 0, 8, 0);
    QLabel *lbl = new QLabel(topFrame);
    int effectCount = element->effects.size();
    if (effectCount == 1) {
        lbl->setText(QString::number(effectCount) + " effect");
    } else {
        lbl->setText(QString::number(effectCount) + " effects");
    }
    lbl->setDisabled(true);
    lbl->setWordWrap(true);
    topLayout->addWidget(lbl);
    topLayout->addStretch();
    QPushButton *addButton = new QPushButton("Add effect", topFrame);
    addButton->setMenu(createEffectsMenu(addButton));
    addButton->setFlat(true);
    topLayout->addWidget(addButton);
    mainLayout->addWidget(topFrame);

    mainLayout->addWidget(new HorizontalLine(this));

    effectUpdateConnection = connect(element, &Element::effectAdded, this,
                                     &EffectsWindow::effectListUpdated);
}

QMenu *EffectsWindow::createEffectsMenu(QWidget *parent) {
    QMenu *menu = new QMenu(parent);

    QAction *action;
    action = menu->addAction("Blur");
    action->setData("blur");
    action = menu->addAction("Invert");
    action->setData("invert");

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
    }

    if (effect) {
        scene->selectedElements[0]->addEffect(effect);
    }
}
