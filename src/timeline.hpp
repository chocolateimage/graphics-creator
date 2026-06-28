#pragma once
#include "scene.hpp"
#include <QWidget>

class TimelineContentWidget : public QWidget {
    const double TIMELINE_HEADER_HEIGHT = 24;
    const double OBJECT_TRACK_HEIGHT = 16;
    const double PROPERTY_TRACK_HEIGHT = 24;

  public:
    explicit TimelineContentWidget(Scene *scene, QWidget *parent = nullptr);
    Scene *scene;
    void updateContents();
    double secondsToPixels();
    double secondsToPixels(double seconds);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};
