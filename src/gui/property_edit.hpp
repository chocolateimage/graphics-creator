#pragma once
#include "animatable/property.hpp"
#include "scene.hpp"
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

class PropertyToggleAnimationButton : public QPushButton {
    Q_OBJECT
  public:
    enum Mode {
        Animation,
        Keyframe,
    };

    explicit PropertyToggleAnimationButton(Mode mode, bool completelyFlat,
                                           Scene *scene, PropertyBase *property,
                                           QWidget *parent = nullptr);
    Scene *scene;
    PropertyBase *property;
    bool completelyFlat;
    Mode mode;

  private slots:
    void toggleAnimationClicked();
    void animationUpdated(PropertyBase *updatedProperty);
    void frameChanged();
};

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

    QSpinBox *inputSpinBox1{nullptr};

  private slots:
    void propertyUpdated(PropertyBase *updatedProperty);
};
