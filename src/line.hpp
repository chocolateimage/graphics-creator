#pragma once

#include <QFrame>

class HorizontalLine : public QFrame {
  public:
    explicit HorizontalLine(QWidget *parent = nullptr) : QFrame(parent) {
        setFrameShape(QFrame::HLine);
        setFrameShadow(QFrame::Sunken);
    }
};
