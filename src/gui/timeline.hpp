#pragma once
#include "scene.hpp"
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

constexpr double TIMELINE_HEADER_HEIGHT = 24;
constexpr double TIMELINE_START_OFFSET = 16;
constexpr double OBJECT_TRACK_HEIGHT = 16;
constexpr double PROPERTY_TRACK_HEIGHT = 16;

class TimelineContentWidget;
class NewMainWindow;

class TimelineWidget : public QWidget {
  public:
    explicit TimelineWidget(Scene *scene, NewMainWindow *mainWindow,
                            QWidget *parent = nullptr);

    TimelineContentWidget *timelineContent;
    QVBoxLayout *timelineLeftLayout;
    QWidget *timelineLeftContents;
    QScrollArea *timelineLeftScrollArea;
    QScrollArea *timelineMainScrollArea;
    QToolButton *playButton;
    Scene *scene;
    NewMainWindow *mainWindow;
    QSlider *zoomSlider;
    void updateContents();
    void timelineScrolled();

  private:
    QPixmap keyframeNo;
    QPixmap keyframeYes;

    void addProperty(PropertyBase *property, bool *stripe,
                     QPushButton *elementButton, int indent);

    void createPixmaps();

  public slots:
    void togglePlay();
    void playbackStateChanged(bool playing);

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

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
    QList<KeyframeBase *> selectedKeyframes;
    bool mouseHeader{false};
    bool selecting{};
    QPoint selectStart;
    QPoint selectEnd;
    QPoint mouseClickStart;
    bool isMovingKeyframes{false};
    bool hasMoved{false};
    bool handleMouseRelease{false};
    QList<int> startKeyframePositions;

    void paintProperty(QPainter &painter, PropertyBase *property, bool *stripe,
                       int startOffset, double *yPos);
    void paintFrameMark(QPainter &painter, int headerPos, int frame);
};
