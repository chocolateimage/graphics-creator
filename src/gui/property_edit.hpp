#pragma once
#include "animatable/property.hpp"
#include "scene.hpp"
#include <QWidget>

class PropertyEdit : public QWidget {
    Q_OBJECT
  public:
    explicit PropertyEdit(PropertyBase *property, Scene *scene,
                          QWidget *parent = nullptr);
    PropertyBase *property;
    Scene *scene;

  private:
    template <typename T> void set(T newValue);

    bool isEditing{false};
    void beginEditing();
    void finishEditing();
    QJsonObject savedState;
};
