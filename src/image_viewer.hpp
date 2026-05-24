#pragma once
#include <QWidget>

class ImageViewer : public QWidget {
    Q_OBJECT
  public:
    explicit ImageViewer(QWidget *parent = nullptr);
    void updateImage(QImage img);
    QRectF fittedRect();
    QImage image;

  protected:
    void clampMovePos();
    QPointF movePos{0, 0};
    QPointF lastDragMousePos;
    float zoom{1};
    bool dragging{false};
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};
