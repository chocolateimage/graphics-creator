#pragma once
#include <QList>
#include <QString>

class ProjectTemplate {
  public:
    QString projectPath;
    QString imagePath;
    QString displayName;
};

void loadProjectTemplates(const QString &dataPath);

extern QList<ProjectTemplate> projectTemplates;
