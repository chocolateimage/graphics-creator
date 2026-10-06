#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

// TODO: transparency
// TODO: hsv inputs
// TODO: palette

class ColorPickerDialog : public QDialog {
    Q_OBJECT
  public:
    explicit ColorPickerDialog(const QColor &color, QWidget *parent = nullptr);

    QColor currentColor;
    QColor originalColor;

    void setColor(const QColor &color);

  private slots:
    void rgbUpdated();
    void hexUpdated(const QString &newHex);

  private:
    QSpinBox *spinR;
    QSpinBox *spinG;
    QSpinBox *spinB;
    QLineEdit *lineHex;
};

class ColorFieldWidget : public QWidget {
    Q_OBJECT
  public:
    explicit ColorFieldWidget(ColorPickerDialog *picker);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    ColorPickerDialog *picker;

    void selectColor(QMouseEvent *event);
    float lastHue = -1;

    bool isMouseInHue = false;
    int poolW;
    int poolH;
    QImage poolImg;
    QImage hueImg;
};

class ColorPreviewWidget : public QWidget {
    Q_OBJECT
  public:
    explicit ColorPreviewWidget(ColorPickerDialog *picker);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    ColorPickerDialog *picker;
};
