#pragma once
#include "animatable/property.hpp"
#include <QWidget>

class TimelineWidget;

struct TimelineKeyframeData {
    KeyframeBase *keyframe;
    double x;
    double y;
    double w;
    double h;
};

class TimelineContentWidget : public QWidget {
  public:
    explicit TimelineContentWidget(TimelineWidget *timelineWidget,
                                   QWidget *parent = nullptr);
    TimelineWidget *timelineWidget;
    void updateContents();
    // Same as update()
    void updatePaint();
    double secondsToPixels();
    double secondsToPixels(double seconds);
    double headerPos();
    bool deleteSelected();
    QList<KeyframeBase *> selectedKeyframes;
    int updatedHeight{0};

  public slots:
    void frameChanged(int frame);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    QList<TimelineKeyframeData> keyframeData;
    bool mouseHeader{false};
    bool selecting{};
    QPoint selectStart;
    QPoint selectEnd;
    QPoint mouseClickStart;
    bool isMovingKeyframes{false};
    bool hasMoved{false};
    bool handleMouseRelease{false};
    QList<int> startKeyframePositions;
    QList<int> startElementMove;
    QList<int> startElementSizes;
    double startMousePosition;
    bool isResizingElements{false};
    bool isResizingOut{false};
    QList<QRectF> elementRects;

    void paintProperty(QPainter &painter, PropertyBase *property, bool *stripe,
                       int startOffset, double *yPos);
    void paintFrameMark(QPainter &painter, int headerPos, int frame);
};
