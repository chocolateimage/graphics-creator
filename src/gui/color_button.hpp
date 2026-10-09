#pragma once
#include <QPushButton>

class ColorButton : public QPushButton {
    Q_OBJECT
  public:
    explicit ColorButton(const QColor &color = {}, QWidget *parent = nullptr);

    void setColor(const QColor &color);
    QColor color() const;

  signals:
    void colorChanged(const QColor &newColor);

  protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;

  private slots:
    void openColorDialog();

  private:
    QColor currentColor;
};
