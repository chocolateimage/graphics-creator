#pragma once
#include "scene.hpp"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

const double TIMELINE_HEADER_HEIGHT = 24;
const double TIMELINE_START_OFFSET = 16;
const double OBJECT_TRACK_HEIGHT = 16;
const double PROPERTY_TRACK_HEIGHT = 16;

class TimelineContentWidget;

class TimelineWidget : public QWidget {
  public:
    explicit TimelineWidget(Scene *scene, QWidget *parent = nullptr);

    TimelineContentWidget *timelineContent;
    QVBoxLayout *timelineLeftLayout;
    QWidget *timelineLeftContents;
    QScrollArea *timelineLeftScrollArea;
    QScrollArea *timelineMainScrollArea;
    Scene *scene;
    void updateContents();
    void timelineScrolled();

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
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
