#pragma once
#include "scene.hpp"
#include "text_element_editor.hpp"
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
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
    enum PickType {
        Point,
        Rect,
    };

    explicit ImageViewer(Scene *scene, QWidget *parent = nullptr);
    void updateImage(QImage img);
    void beginPicking(const QString &id, const QString &infoText,
                      PickType pickType);
    void stopPicking();
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
    PickType pickType;
    QString pickId;
    QString pickText;
    QPoint startPickPosition;
    QPoint pickPosition;
    QPoint getActualPickPosition();
    void updateCursor();
    Scene *scene;

    QPointF pixelToViewport(QPointF pos);
    QPoint viewportToPixel(QPointF pos);

    bool isMovingElements{false};
    QPoint startMovePosition;

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;

    TextElementEditor *textElementEditor{nullptr};

  private slots:
    void elementSelectionChanged(QList<Element *> elements);
    void elementEditModeChanged(Element *element, bool editMode);
    void playbackStateChanged(bool playing);

  signals:
    void pixelPicked(QString id, QPoint position);
    void rectPicked(QString id, QRect rect);
};
