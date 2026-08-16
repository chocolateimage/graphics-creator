#pragma once
#include <QMultiMap>
#include <QMutex>
#include <QWaitCondition>

class PriorityMutex {
  public:
    void lock(double seconds);
    void unlock();

    QMutex mutex;
    QWaitCondition waitCondition;
    QMultiMap<double, uint64_t> map;
    bool locked{false};
    uint64_t id{0};
};

class PriorityMutexLocker {
  public:
    PriorityMutexLocker(PriorityMutex *mutex, double seconds);
    ~PriorityMutexLocker();

    PriorityMutex *mutex;
};
