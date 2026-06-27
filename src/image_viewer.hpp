#pragma once
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QToolButton>
#include <QWidget>

class TransparentCornerFrame : public QFrame {
    Q_OBJECT
  public:
    explicit TransparentCornerFrame(QWidget *parent = nullptr);
    QGraphicsOpacityEffect *opacityEffect;

  protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

class ImageViewer : public QWidget {
    Q_OBJECT
  public:
    explicit ImageViewer(QWidget *parent = nullptr);
    void updateImage(QImage img);
    void beginPicking(const QString &id, const QString &infoText);
    QRectF fittedRect();
    QImage image;

  protected:
    void saveFrameAsFile(QSize size);
    TransparentCornerFrame *cornerFrame;
    QToolButton *windowButton;
    bool darkCheckerboard{true};
    void clampMovePos();
    QPointF movePos{0, 0};
    QPointF lastDragMousePos;
    float zoom{1};
    bool dragging{false};
    bool isPicking{false};
    QString pickId;
    QString pickText;
    QPoint pickPosition;
    void updateCursor();
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

  signals:
    void pixelPicked(QString id, QPoint position);
};
