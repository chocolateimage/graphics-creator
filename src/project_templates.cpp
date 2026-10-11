#include "project_templates.hpp"
#include <QByteArray>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QList<ProjectTemplate> projectTemplates = {};

void loadProjectTemplates(const QString &dataPath) {
    QFile templatesFile(dataPath + "/project-templates/templates.json");
    if (templatesFile.open(QIODevice::ReadOnly)) {
        QByteArray templatesData = templatesFile.readAll();
        QJsonDocument templatesDoc = QJsonDocument::fromJson(templatesData);
        templatesFile.close();

        for (const auto &templateJson : templatesDoc.array()) {
            auto templateObj = templateJson.toObject();
            QString fileName = dataPath + "/project-templates/" +
                               templateObj["file"].toString();

            projectTemplates.append(ProjectTemplate{
                .projectPath = fileName + ".gcp",
                .imagePath = fileName + ".png",
                .displayName = templateObj["name"].toString(),
            });
        }
    }
}
