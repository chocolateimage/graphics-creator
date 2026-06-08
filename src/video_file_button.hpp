#pragma once

#include <QPushButton>

class VideoFileButton : public QPushButton {
    Q_OBJECT
  public:
    explicit VideoFileButton(QWidget *parent = nullptr);

    QString filePath;

    void setFile(QString filePath);
    void openFile();

    QPoint dragStartPosition;

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};
