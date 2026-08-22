#include "log.hpp"
#include <iostream>

void logJson(const QJsonObject &obj) {
    QJsonDocument doc;
    doc.setObject(obj);
    std::cout << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
}
