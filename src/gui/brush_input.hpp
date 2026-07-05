#pragma once

#include "variant.hpp"
#include <KColorButton>
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

  signals:
    void valueChanged(Brush value);

  private:
    void updateType();
    void showMenu();
    Brush::Type getBrushType();
    QAction *actionSingleColor;
    QAction *actionLinearGradient;
    QAction *actionRadialGradient;
    KColorButton *color1;
    KColorButton *color2;
    QSpinBox *angleInput;
    QPushButton *changeButton;
    QMenu *typeMenu;
};
