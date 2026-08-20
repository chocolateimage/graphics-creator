#include "priority_mutex.hpp"

void PriorityMutex::lock(double seconds) {
    QMutexLocker locker(&mutex);
    uint64_t thisId = id++;
    map.insert(seconds, thisId);
    while (locked || map.first() != thisId) {
        waitCondition.wait(&mutex);
    }
    map.remove(seconds, thisId);
    locked = true;
}
void PriorityMutex::unlock() {
    QMutexLocker locker(&mutex);
    locked = false;
    waitCondition.wakeAll();
}

PriorityMutexLocker::PriorityMutexLocker(PriorityMutex *mutex, double seconds)
    : mutex(mutex) {
    mutex->lock(seconds);
}

PriorityMutexLocker::~PriorityMutexLocker() { mutex->unlock(); }
