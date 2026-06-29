#pragma once
#include "scene.hpp"
#include <QWidget>

const double TIMELINE_HEADER_HEIGHT = 24;
const double OBJECT_TRACK_HEIGHT = 16;
const double PROPERTY_TRACK_HEIGHT = 24;

class TimelineContentWidget;

class TimelineWidget : public QWidget {
  public:
    explicit TimelineWidget(Scene *scene, QWidget *parent = nullptr);

    TimelineContentWidget *timelineContent;
    Scene *scene;
    void updateContents();
};

class TimelineContentWidget : public QWidget {
  public:
    explicit TimelineContentWidget(TimelineWidget *timelineWidget,
                                   QWidget *parent = nullptr);
    TimelineWidget *timelineWidget;
    void updateContents();
    double secondsToPixels();
    double secondsToPixels(double seconds);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};
