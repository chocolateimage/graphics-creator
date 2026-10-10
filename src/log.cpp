#include "log.hpp"
#include <QMutex>
#include <iostream>

QMutex logJsonLock;

void logJson(const QJsonObject &obj) {
    QMutexLocker lock(&logJsonLock);
    QJsonDocument doc;
    doc.setObject(obj);
    std::cout << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
}
