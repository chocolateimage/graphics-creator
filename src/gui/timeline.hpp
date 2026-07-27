#pragma once
#include "scene.hpp"
#include <QLabel>
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

class TimelineWidget;
class TimelineContentWidget;
class NewMainWindow;

class TimelineElementButton : public QPushButton {
  public:
    explicit TimelineElementButton(Element *element,
                                   TimelineWidget *timelineWidget);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private slots:
    void elementNameChanged(const QString &objectName);
    void collapseClicked();
    void clickedSlot();
    void visibilityClicked();
    void visibilityUpdated();

  private:
    TimelineWidget *timelineWidget;
    Element *element;
    QPoint dragStartPosition;
    QLabel *objectNameLabel;
    QPushButton *visibilityButton;
};

class TimelineWidget : public QWidget {
  public:
    explicit TimelineWidget(Scene *scene, NewMainWindow *mainWindow,
                            QWidget *parent = nullptr);

    TimelineContentWidget *timelineContent;
    QVBoxLayout *timelineLeftLayout;
    QWidget *timelineLeftContents;
    QScrollArea *timelineLeftScrollArea;
    QScrollArea *timelineMainScrollArea;
    QPushButton *playButton;
    Scene *scene;
    NewMainWindow *mainWindow;
    QSlider *zoomSlider;
    QFrame *elementMoveBar{nullptr};
    int elementMoveTarget;
    QList<TimelineElementButton *> elementButtons;
    void updateContents();
    void timelineScrolled();

  private:
    QPixmap keyframeNo;
    QPixmap keyframeYes;

    bool addCollapsible(ICollapsible *collapsible, bool *stripe, bool selected);
    void addProperty(PropertyBase *property, bool *stripe,
                     QPushButton *elementButton, int indent, bool showEdit);

    void createPixmaps();

  public slots:
    void togglePlay();
    void goToStart();
    void playbackStateChanged(bool playing);
    void elementSelectionChanged(QList<Element *> elements);

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
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

    void paintProperty(QPainter &painter, PropertyBase *property, bool *stripe,
                       int startOffset, double *yPos);
    void paintFrameMark(QPainter &painter, int headerPos, int frame);
};
