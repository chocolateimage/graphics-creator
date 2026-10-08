#pragma once

#include "color_button.hpp"
#include "variant.hpp"
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

class BrushInput : public QWidget {
    Q_OBJECT
  public:
    explicit BrushInput(QWidget *parent = nullptr);

    Brush value();

  public slots:
    void setValue(Brush value);

  private slots:
    void _valueChanged();
    void _valueChangedFinished();

  signals:
    void valueChanged(Brush value);
    void editingFinished();

  private:
    void updateType();
    void showMenu();
    Brush::Type getBrushType();
    QAction *actionSingleColor;
    QAction *actionLinearGradient;
    QAction *actionRadialGradient;
    ColorButton *color1;
    ColorButton *color2;
    QSpinBox *angleInput;
    QPushButton *changeButton;
    QMenu *typeMenu;
};
