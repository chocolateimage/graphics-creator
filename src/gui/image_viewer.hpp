#pragma once
#include "scene.hpp"
#include "text_element_editor.hpp"
#include <DockManager.h>
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

class NewMainWindow;

struct ResizeMode {
    // resizing itself
    int moveX;
    int moveY;
    int sizeX;
    int sizeY;

    // Which side you can click and drag 0, 0.5, 1
    float sideX;
    float sideY;
    Qt::CursorShape cursor;
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

    NewMainWindow *mainWindow{nullptr};

  protected:
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
    QRect getPickRect();
    void updateCursor();
    Scene *scene;

    std::vector<ResizeMode> resizeModes;

    QPointF pixelToViewport(QPointF pos);
    QPoint viewportToPixel(QPointF pos);

    bool isMovingElements{false};
    bool didMove{false};
    QPoint startMovePosition;
    QList<QPoint> startElementPositions;
    QList<QPoint> startElementBoundPositions;
    QRect snapVisualRect1;
    QRect snapVisualRect2;
    QList<QJsonObject> moveOlds;
    QList<QJsonObject> moveNews;
    int moveSpacing{0};
    QPoint moveSpacingCursorStart;
    bool moveSpacingMoving{false};
    QRect snapElementRectPreview;
    Element *hoverElement{nullptr};

    bool isDroppingImage{false};
    QString dropImagePath;
    QImage dropImagePreview;
    QPoint dropImageCursor;
    QSize dropImageSize;
    bool dropImageFitted{false};

    Element *hoverResizeElement;
    int hoverResizeMode{-1};
    int activeResizeMode{-1};
    QPoint startResizePosition;
    QRect startResizeRect;
    Element *resizeElement;
    QJsonObject resizeOldX;
    QJsonObject resizeOldY;
    QJsonObject resizeOldW;
    QJsonObject resizeOldH;

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
    void inputMethodEvent(QInputMethodEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool event(QEvent *event) override;

    void paintSnapVisualRect(QPainter &painter, const QRect &snapVisualRect);

    TextElementEditor *textElementEditor{nullptr};

  private slots:
    void elementSelectionChanged(QList<Element *> elements);
    void elementEditModeChanged(Element *element, bool editMode);
    void playbackStateChanged(bool playing);

  signals:
    void pixelPicked(QString id, QPoint position);
    void rectPicked(QString id, QRect rect);
};
