#include "brush_input.hpp"
#include <KColorButton>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QMenu>
#include <QToolButton>

BrushInput::BrushInput(QWidget *parent) : QWidget(parent) {
    auto lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    color1 = new KColorButton(this);
    lay->addWidget(color1);
    connect(color1, &KColorButton::changed, this, &BrushInput::_valueChanged);
    color2 = new KColorButton(this);
    connect(color2, &KColorButton::changed, this, &BrushInput::_valueChanged);
    lay->addWidget(color2);

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
}

void BrushInput::showMenu() {
    typeMenu->exec(
        changeButton->mapToGlobal(changeButton->rect().bottomLeft()));
    updateType();
    _valueChanged();
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
    actionSingleColor->setChecked(value.brushType == Brush::SingleColor);
    actionLinearGradient->setChecked(value.brushType == Brush::LinearGradient);
    actionRadialGradient->setChecked(value.brushType == Brush::RadialGradient);
    color1->setColor(QColor::fromRgb(value.color1.r, value.color1.g,
                                     value.color1.b, value.color1.a));
    color2->setColor(QColor::fromRgb(value.color2.r, value.color2.g,
                                     value.color2.b, value.color2.a));
    updateType();
}

void BrushInput::_valueChanged() { emit valueChanged(value()); }

Brush BrushInput::value() {
    Brush brush;
    brush.brushType = getBrushType();
    QColor color = color1->color();
    brush.color1 = {color.red(), color.green(), color.blue(), color.alpha()};
    color = color2->color();
    brush.color2 = {color.red(), color.green(), color.blue(), color.alpha()};

    return brush;
}
