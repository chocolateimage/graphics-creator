#pragma once
#include <QPushButton>

class QMimeData;

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
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  private slots:
    void openColorDialog();

  private:
    QColor colorFromMime(const QMimeData *mimeData);
    QString hexTransparency();
    QColor currentColor;
    QPointF dragStartPosition;
};
