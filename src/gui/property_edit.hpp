#pragma once
#include "animatable/property.hpp"
#include <QWidget>

class PropertyEdit : public QWidget {
    Q_OBJECT
  public:
    explicit PropertyEdit(PropertyBase *property, QWidget *parent = nullptr);
    PropertyBase *property;

  private:
    template <typename T> void set(T newValue);
};
