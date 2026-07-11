#pragma once
#include "animatable/element/element.hpp"
#include <QChronoTimer>
#include <QElapsedTimer>
#include <QList>
#include <QObject>

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
    int currentFrame{0};
    void startTimer();
    void stopTimer();
    void timerTicked();

    void addElement(Element *element);
    void insertElement(Element *element, int index);
    void selectElements(QList<Element *> elements);

  signals:
    void elementAdded(Element *element, int index);
    void elementRemoved(Element *element);
    void elementUpdated(Element *element);
    void elementSelectionChanged(QList<Element *> elements);
    void frameChanged(int frame);
    void playbackStateChanged(bool playing);

  private:
    QChronoTimer *timer;
    int startFrame;
    QElapsedTimer elapsedTimer;
};
