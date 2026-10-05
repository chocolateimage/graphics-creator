#include "color_picker.hpp"
#include <QApplication>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

ColorPickerDialog::ColorPickerDialog(const QColor &color, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Pick Color");
    originalColor = color;

    QVBoxLayout *mainLay = new QVBoxLayout(this);

    QHBoxLayout *lay = new QHBoxLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(16);
    mainLay->addLayout(lay);

    ColorFieldWidget *fieldWidget = new ColorFieldWidget(this);
    lay->addWidget(fieldWidget);

    QVBoxLayout *lay2 = new QVBoxLayout();
    lay2->setSpacing(4);
    lay2->setContentsMargins(0, 0, 0, 0);
    ColorPreviewWidget *previewWidget = new ColorPreviewWidget(this);
    lay2->addWidget(previewWidget);

    QFormLayout *formLay = new QFormLayout();
    spinR = new QSpinBox();
    spinG = new QSpinBox();
    spinB = new QSpinBox();
    lineHex = new QLineEdit();
    lineHex->setMaxLength(6);
    spinR->setRange(0, 255);
    spinG->setRange(0, 255);
    spinB->setRange(0, 255);
    spinR->setSizePolicy(QSizePolicy::Policy::Expanding,
                         QSizePolicy::Policy::Fixed);
    spinG->setSizePolicy(QSizePolicy::Policy::Expanding,
                         QSizePolicy::Policy::Fixed);
    spinB->setSizePolicy(QSizePolicy::Policy::Expanding,
                         QSizePolicy::Policy::Fixed);
    connect(spinR, &QSpinBox::valueChanged, this,
            &ColorPickerDialog::rgbUpdated);
    connect(spinG, &QSpinBox::valueChanged, this,
            &ColorPickerDialog::rgbUpdated);
    connect(spinB, &QSpinBox::valueChanged, this,
            &ColorPickerDialog::rgbUpdated);
    lineHex->setFixedWidth(80);
    formLay->addRow("R", spinR);
    formLay->addRow("G", spinG);
    formLay->addRow("B", spinB);
    formLay->addRow("#", lineHex);
    lay2->addLayout(formLay);

    QPushButton *test = new QPushButton("...");
    connect(test, &QPushButton::clicked, this, [this]() {
        QColorDialog dialog;
        dialog.setCurrentColor(currentColor);
        if (dialog.exec() == QDialog::DialogCode::Accepted) {
            setColor(dialog.currentColor());
        }
    });
    lay2->addWidget(test);

    lay->addLayout(lay2);

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this,
            &ColorPickerDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this,
            &ColorPickerDialog::reject);
    mainLay->addWidget(buttonBox);
    setColor(color);

    QTimer::singleShot(0, this, [this]() { setFixedSize(size()); });
}

void ColorPickerDialog::setColor(const QColor &color) {
    QSignalBlocker block1(spinR);
    QSignalBlocker block2(spinG);
    QSignalBlocker block3(spinB);
    QSignalBlocker block4(lineHex);
    currentColor = color.convertTo(QColor::Hsv);
    spinR->setValue(currentColor.red());
    spinG->setValue(currentColor.green());
    spinB->setValue(currentColor.blue());
    lineHex->setText(currentColor.name().sliced(1));
    update();
}

void ColorPickerDialog::rgbUpdated() {
    setColor(QColor(spinR->value(), spinG->value(), spinB->value()));
}

ColorFieldWidget::ColorFieldWidget(ColorPickerDialog *picker) : picker(picker) {
    poolW = 250;
    poolH = 250;
    int hueW = 20;
    int hueH = poolH;
    setFixedSize(poolW + 10 + hueW, poolH);

    hueImg = QImage(hueW, hueH, QImage::Format_RGB32);
    QColor color;
    for (int y = 0; y < hueH; y++) {
        color.setHsvF((float)y / hueH, 1, 1);
        for (int x = 0; x < hueW; x++) {
            hueImg.setPixelColor(x, y, color);
        }
    }
}

void ColorFieldWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    float hue = picker->currentColor.hueF();
    if (hue == -1)
        hue = 0;
    QColor color;
    if (lastHue != hue) {
        lastHue = hue;
        poolImg = QImage(poolW, poolH, QImage::Format_RGB32);
        for (int y = 0; y < poolH; y++) {
            for (int x = 0; x < poolW; x++) {
                float xa = (float)x / poolW;
                float ya = (float)y / poolH;
                color.setHsvF(hue, xa, 1 - ya);
                poolImg.setPixelColor(x, y, color);
            }
        }
    }
    painter.drawImage(0, 0, poolImg);
    color = picker->currentColor;
    bool isDarkColor =
        (color.redF() * 0.2 + color.greenF() * 0.7 + color.blueF() * 0.1) < 0.5;
    painter.setPen(QPen(isDarkColor ? Qt::white : Qt::black, 2));
    painter.setClipRect(0, 0, poolW, poolH);
    painter.drawEllipse(
        QPoint(color.saturationF() * poolW, (1 - color.valueF()) * poolH), 6,
        6);
    painter.setClipping(false);

    int hueX = poolW + 10;
    painter.drawImage(hueX, 0, hueImg);

    qreal hueCenterY = hue * poolH;
    QPainterPath path;
    path.moveTo(hueX + .5, hueCenterY - 6);
    path.lineTo(hueX + .5 + 6, hueCenterY);
    path.lineTo(hueX + .5, hueCenterY + 6);
    path.lineTo(hueX + .5, hueCenterY - 6);
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::white);
    painter.drawPath(path);
}

void ColorFieldWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int hueX = poolW + 10;
        int hueW = 20;
        float x = event->position().x();
        if (x < poolW) {
            isMouseInHue = false;
        } else if (x >= hueX && x < hueX + hueW) {
            isMouseInHue = true;
        }
    }
    selectColor(event);
}
void ColorFieldWidget::mouseMoveEvent(QMouseEvent *event) {
    selectColor(event);
}
void ColorFieldWidget::mouseReleaseEvent(QMouseEvent *event) {}

void ColorFieldWidget::selectColor(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;

    float x = event->position().x();
    float y = event->position().y();

    if (isMouseInHue) {
        float ya = std::clamp(y / poolH, 0.f, 1.f);
        picker->setColor(QColor::fromHsvF(ya,
                                          picker->currentColor.saturationF(),
                                          picker->currentColor.valueF()));
    } else {
        float xa = std::clamp(x / poolW, 0.f, 1.f);
        float ya = std::clamp(y / poolH, 0.f, 1.f);
        picker->setColor(
            QColor::fromHsvF(picker->currentColor.hueF(), xa, 1 - ya));
    }
}

ColorPreviewWidget::ColorPreviewWidget(ColorPickerDialog *picker)
    : picker(picker) {
    setFixedHeight(60);
}

void ColorPreviewWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(picker->currentColor);
    painter.drawRect(0, 0, width(), height() / 2);
    painter.setBrush(picker->originalColor);
    painter.drawRect(0, height() / 2, width(), height() / 2);
}
