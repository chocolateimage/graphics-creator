#pragma once
#include "scene.hpp"
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

constexpr double TIMELINE_HEADER_HEIGHT = 24;
constexpr double TIMELINE_START_OFFSET = 16;
constexpr double OBJECT_TRACK_HEIGHT = 16;
constexpr double PROPERTY_TRACK_HEIGHT = 16;

class TimelineContentWidget;

class TimelineWidget : public QWidget {
  public:
    explicit TimelineWidget(Scene *scene, QWidget *parent = nullptr);

    TimelineContentWidget *timelineContent;
    QVBoxLayout *timelineLeftLayout;
    QWidget *timelineLeftContents;
    QScrollArea *timelineLeftScrollArea;
    QScrollArea *timelineMainScrollArea;
    QToolButton *playButton;
    Scene *scene;
    void updateContents();
    void timelineScrolled();

  public slots:
    void togglePlay();
    void playbackStateChanged(bool playing);

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

  public slots:
    void frameChanged(int frame);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};
