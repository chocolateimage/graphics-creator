#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

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
    QSpinBox *spinA;
    QLineEdit *lineHex;
};

class ColorFieldWidget : public QWidget {
    Q_OBJECT
    enum MouseHover {
        None,
        Pool,
        Hue,
        Transparency,
    };

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

    MouseHover hover;
    int poolW;
    int poolH;
    int hueX;
    int hueW;
    int hueH;
    int transparencyX;
    QImage poolImg;
    QImage hueImg;
    QImage transparencyImg;
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
