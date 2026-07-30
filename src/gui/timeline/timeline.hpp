#pragma once
#include "scene.hpp"

#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

constexpr double TIMELINE_HEADER_HEIGHT = 24;
constexpr double TIMELINE_START_OFFSET = 16;
constexpr double OBJECT_TRACK_HEIGHT = 16;
constexpr double PROPERTY_TRACK_HEIGHT = 16;

static const QString ELEMENT_DRAG_MIME_TYPE =
    "application/x-graphicscreator-element-drag";

class NewMainWindow;
class TimelineContentWidget;
class TimelineElementButton;

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
    void framesChanging(bool changing);

    bool freezeTimelineScroll{false};

    QPixmap keyframeNo;
    QPixmap keyframeYes;

  private:
    bool addCollapsible(ICollapsible *collapsible, bool *stripe, bool selected,
                        int indent);
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
