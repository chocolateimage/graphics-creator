#pragma once
#include <QDoubleSpinBox>

class DraggableSpinBox : public QSpinBox {
  public:
    explicit DraggableSpinBox(QWidget *parent = nullptr);

  protected:
    int holdStartValue;
    QPoint holdCompleteStartPos;
    QPoint holdAfterDragStartPos;
    QPointF holdStartPos;
    bool isHolding{false};
    bool isDragging{false};
    bool eventFilter(QObject *obj, QEvent *event) override;
};

class DraggableDoubleSpinBox : public QDoubleSpinBox {
  public:
    explicit DraggableDoubleSpinBox(QWidget *parent = nullptr);

  protected:
    double holdStartValue;
    QPoint holdCompleteStartPos;
    QPoint holdAfterDragStartPos;
    QPointF holdStartPos;
    bool isHolding{false};
    bool isDragging{false};
    bool eventFilter(QObject *obj, QEvent *event) override;
};
