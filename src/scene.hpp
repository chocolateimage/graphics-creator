#pragma once
#include "animatable/element/element.hpp"
#include <QChronoTimer>
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QUndoStack>

class Scene : public QObject {
    Q_OBJECT
  public:
    Scene();
    ~Scene();
    int width;
    int height;
    float frameRate;
    int durationFrames;
    QList<Element *> elements;
    QList<Element *> selectedElements;
    QUndoStack *undoStack;
    int currentFrame{0};
    bool isPlaying();
    void startTimer();
    void stopTimer();
    void timerTicked();
    void setFramesChanging(bool changing);
    void setFrame(int frame);

    void addElement(Element *element);
    void removeElement(Element *element);
    void insertElement(Element *element, int index);
    void reorderElement(Element *element, int newIndex);
    void selectElements(QList<Element *> elements);

    std::function<bool()> canContinuePlayback;

  signals:
    void elementAdded(Element *element, int index);
    void elementUpdated(Element *element);
    void elementRemoved(Element *element);
    void elementOrderChanged();
    void elementSelectionChanged(QList<Element *> elements);
    void elementEditModeChanged(Element *element, bool editMode);
    void sceneInfoChanged();
    void frameChanged(int frame);
    void playbackStateChanged(bool playing);

    // Called true when frames are rapidly being changed
    // (during playback/when scrubbing). Called false when it stops.
    void framesChanging(bool changing);

  private slots:
    void _elementUpdatedSlot();
    void _elementEditModeChangedSlot(bool editMode);

  private:
    QChronoTimer *timer;
    int startFrame;
    QElapsedTimer elapsedTimer;
};
