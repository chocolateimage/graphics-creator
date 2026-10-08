#include "brush_input.hpp"
#include "draggable_spinbox.hpp"
#include <QActionGroup>
#include <QHBoxLayout>
#include <QMenu>
#include <QToolButton>

BrushInput::BrushInput(QWidget *parent) : QWidget(parent) {
    auto lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    color1 = new ColorButton();
    lay->addWidget(color1);
    connect(color1, &ColorButton::colorChanged, this,
            &BrushInput::_valueChangedFinished);
    color2 = new ColorButton();
    connect(color2, &ColorButton::colorChanged, this,
            &BrushInput::_valueChangedFinished);
    lay->addWidget(color2);

    angleInput = new DraggableSpinBox(this);
    angleInput->setSuffix("°");
    angleInput->setMinimum(0);
    angleInput->setMaximum(360);
    connect(angleInput, &QSpinBox::valueChanged, this,
            &BrushInput::_valueChanged);
    connect(angleInput, &QSpinBox::editingFinished, this,
            &BrushInput::editingFinished);
    lay->addWidget(angleInput);

    typeMenu = new QMenu(this);

    QActionGroup *group = new QActionGroup(typeMenu);
    group->setExclusive(true);

    actionSingleColor = group->addAction("Single Color");
    actionSingleColor->setCheckable(true);
    actionLinearGradient = group->addAction("Linear Gradient");
    actionLinearGradient->setCheckable(true);
    actionRadialGradient = group->addAction("Radial Gradient");
    actionRadialGradient->setCheckable(true);

    typeMenu->addAction(actionSingleColor);
    typeMenu->addAction(actionLinearGradient);
    typeMenu->addAction(actionRadialGradient);

    changeButton = new QPushButton(this);
    changeButton->setIcon(QIcon::fromTheme("arrow-down"));
    connect(changeButton, &QPushButton::pressed, this, &BrushInput::showMenu);
    lay->addWidget(changeButton);
}

void BrushInput::updateType() {
    Brush::Type brushType = getBrushType();

    color1->setVisible(brushType == Brush::SingleColor ||
                       brushType == Brush::LinearGradient ||
                       brushType == Brush::RadialGradient);

    color2->setVisible(brushType == Brush::LinearGradient ||
                       brushType == Brush::RadialGradient);

    angleInput->setVisible(brushType == Brush::LinearGradient);
}

void BrushInput::showMenu() {
    typeMenu->exec(
        changeButton->mapToGlobal(changeButton->rect().bottomLeft()));
    updateType();
    _valueChangedFinished();
    changeButton->clearFocus();
}

Brush::Type BrushInput::getBrushType() {
    if (actionSingleColor->isChecked()) {
        return Brush::SingleColor;
    } else if (actionLinearGradient->isChecked()) {
        return Brush::LinearGradient;
    } else if (actionRadialGradient->isChecked()) {
        return Brush::RadialGradient;
    }

    return Brush::SingleColor; // fallback
}

void BrushInput::setValue(Brush value) {
    QSignalBlocker blocker(color1);
    QSignalBlocker blocker2(color2);
    QSignalBlocker blocker3(angleInput);
    actionSingleColor->setChecked(value.brushType == Brush::SingleColor);
    actionLinearGradient->setChecked(value.brushType == Brush::LinearGradient);
    actionRadialGradient->setChecked(value.brushType == Brush::RadialGradient);
    color1->setColor(value.color1.toQColor());
    color2->setColor(value.color2.toQColor());
    angleInput->setValue(value.angle);
    updateType();
}

void BrushInput::_valueChanged() { emit valueChanged(value()); }
void BrushInput::_valueChangedFinished() {
    emit valueChanged(value());
    emit editingFinished();
}

Brush BrushInput::value() {
    Brush brush;
    brush.brushType = getBrushType();
    QColor color = color1->color();
    brush.color1 = {color.red(), color.green(), color.blue(), color.alpha()};
    color = color2->color();
    brush.color2 = {color.red(), color.green(), color.blue(), color.alpha()};
    brush.angle = angleInput->value();

    return brush;
}
