#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

// TODO: palette

class QColorDialog;

class ColorPickerDialog : public QDialog {
    Q_OBJECT
  public:
    explicit ColorPickerDialog(const QColor &color, QWidget *parent = nullptr);

    QColor currentColor;
    QColor originalColor;

    void setColor(const QColor &color);

  private slots:
    void rgbUpdated();
    void hsvUpdated();
    void hexUpdated(const QString &newHex);
    void pickColor();

  protected:
    bool eventFilter(QObject *, QEvent *) override;

  private:
    QSpinBox *spinR;
    QSpinBox *spinG;
    QSpinBox *spinB;
    QSpinBox *spinA;
    QSpinBox *spinH;
    QSpinBox *spinS;
    QSpinBox *spinV;
    QLineEdit *lineHex;
    QColorDialog *colorDialog{nullptr};
    QPushButton *colorDialogButton{nullptr};
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
