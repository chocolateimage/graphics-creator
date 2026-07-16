#include "property_edit.hpp"
#include "brush_input.hpp"
#include "draggable_spinbox.hpp"
#include "fontcombobox.hpp"
#include "line.hpp"
#include "math.hpp"
#include <KColorButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QPlainTextEdit>
#include <QSlider>
#include <QToolButton>

PropertyEdit::PropertyEdit(PropertyBase *property, Scene *scene,
                           QWidget *parent)
    : QWidget(parent), property(property), scene(scene) {
    auto lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    auto variant = property->toVariant({scene->currentFrame});
    auto variantType = variant.type();
    QWidget *widget{nullptr};
    int min = INT_MIN;
    int max = INT_MAX;

    if (variantType == VariantTypeEnum::None) {
        auto line = new HorizontalLine(this);
        line->setSizePolicy(QSizePolicy::Policy::Expanding,
                            QSizePolicy::Policy::Minimum);
        widget = line;
    } else if (variantType == VariantTypeEnum::String) {
        auto input = new QPlainTextEdit(this);
        input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        input->setFixedHeight(50);
        input->setPlainText(QString::fromStdString(variant.get<std::string>()));
        connect(input, &QPlainTextEdit::textChanged, this,
                [this, input]() { set(input->toPlainText().toStdString()); });
        widget = input;
    } else if (variantType == VariantTypeEnum::Int) {
        bool hasSlider = false;

        auto propertyTyped = (Property<int> *)property;

        if (propertyTyped->hasMin) {
            min = propertyTyped->min;
        }
        if (propertyTyped->hasMax) {
            max = propertyTyped->max;
        }

        if (hasSlider) {
            auto input = new QSlider(this);
            input->setOrientation(Qt::Horizontal);
            input->setMinimum(min);
            input->setMaximum(max);
            input->setValue(variant.get<int>());
            connect(input, &QSlider::valueChanged, this,
                    [this](int value) { set(value); });
            widget = input;
        } else {
            auto input = new DraggableSpinBox(this);
            input->setMinimum(min);
            input->setMaximum(max);
            input->setValue(variant.get<int>());
            connect(input, &DraggableSpinBox::valueChanged, this,
                    [this](int value) { set(value); });
            widget = input;
        }
    } else if (variantType == VariantTypeEnum::Double) {
        auto input = new DraggableDoubleSpinBox(this);

        auto propertyTyped = (Property<double> *)property;
        if (propertyTyped->hasMin) {
            min = propertyTyped->min;
        }
        if (propertyTyped->hasMax) {
            max = propertyTyped->max;
        }

        input->setMinimum(min);
        input->setMaximum(max);
        input->setValue(variant.get<double>());
        connect(input, &DraggableDoubleSpinBox::valueChanged, this,
                [this](double value) { set(value); });
        widget = input;
    } else if (variantType == VariantTypeEnum::Vector2DInt) {
        widget = new QWidget(this);
        auto layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        auto value = variant.get<Vector2DInt>();

        auto inputX = new DraggableSpinBox(this);
        inputX->setMinimum(min);
        inputX->setMaximum(max);
        inputX->setValue(value.x);

        auto inputY = new DraggableSpinBox(this);
        inputY->setMinimum(min);
        inputY->setMaximum(max);
        inputY->setValue(value.y);

        auto pickButton = new QToolButton(this);
        pickButton->setToolTip("Pick position from preview");
        pickButton->setIcon(QIcon::fromTheme("select"));

        layout->addWidget(inputX);
        layout->addWidget(inputY);
        layout->addWidget(pickButton);

        connect(inputX, &DraggableSpinBox::valueChanged, this,
                [this, inputY](int value) {
                    set((Vector2DInt){value, inputY->value()});
                });
        connect(inputY, &DraggableSpinBox::valueChanged, this,
                [this, inputX](int value) {
                    set((Vector2DInt){inputX->value(), value});
                });
        // connect(pickButton, &QToolButton::clicked, this, [this,
        // optionLabel]() {
        //     previewWidget->beginPicking(QString::fromStdString(optionId),
        //                                 "Pick \"" + optionLabel + "\"",
        //                                 ImageViewer::PickType::Point);
        // });
    } else if (variantType == VariantTypeEnum::Color) {
        auto colorButton = new KColorButton(this);
        colorButton->setAlphaChannelEnabled(true);
        auto value = variant.get<Color>();
        colorButton->setColor(QColor(value.r, value.g, value.b, value.a));
        connect(colorButton, &KColorButton::changed, this,
                [this](const QColor &newColor) {
                    set((Color){newColor.red(), newColor.green(),
                                newColor.blue(), newColor.alpha()});
                });
        widget = colorButton;
    } else if (variantType == VariantTypeEnum::Font) {
        auto fontWidget = new FontComboBox(this);
        fontWidget->setSizePolicy(QSizePolicy::Policy::Expanding,
                                  QSizePolicy::Policy::Fixed);
        auto value = variant.get<Font>();
        if (!value.path.empty()) {
            fontWidget->setFontValue(value);
        }
        connect(fontWidget, &FontComboBox::currentTextChanged, this,
                [this, fontWidget]() { set(fontWidget->fontValue()); });

        widget = fontWidget;
    } else if (variantType == VariantTypeEnum::Bool) {
        auto input = new QCheckBox(this);
        input->setChecked(variant.get<bool>());
        connect(input, &QCheckBox::checkStateChanged, this,
                [this](Qt::CheckState checkState) {
                    set(checkState == Qt::CheckState::Checked);
                });
        widget = input;
    } else if (variantType == VariantTypeEnum::Easing) {
        std::vector<std::function<double(double)>> functions = {
            linear,         easeInQuad,     easeOutQuad,    easeInOutQuad,
            easeInCubic,    easeOutCubic,   easeInOutCubic, easeInQuart,
            easeOutQuart,   easeInOutQuart, easeInQuint,    easeOutQuint,
            easeInOutQuint, easeInSine,     easeOutSine,    easeInOutSine,
            easeInExpo,     easeOutExpo,    easeInOutExpo,  easeInCirc,
            easeOutCirc,    easeInOutCirc,  easeInBack,     easeOutBack,
            easeInOutBack,  easeInElastic,  easeOutElastic, easeInOutElastic,
            easeInBounce,   easeOutBounce,  easeInOutBounce};

        std::vector<std::string> names = {"",

                                          "easeInQuad",
                                          "easeOutQuad",
                                          "easeInOutQuad",
                                          "easeInCubic",
                                          "easeOutCubic",
                                          "easeInOutCubic",
                                          "easeInQuart",
                                          "easeOutQuart",
                                          "easeInOutQuart",
                                          "easeInQuint",
                                          "easeOutQuint",
                                          "easeInOutQuint",
                                          "easeInSine",
                                          "easeOutSine",
                                          "easeInOutSine",
                                          "easeInExpo",
                                          "easeOutExpo",
                                          "easeInOutExpo",
                                          "easeInCirc",
                                          "easeOutCirc",
                                          "easeInOutCirc",
                                          "easeInBack",
                                          "easeOutBack",
                                          "easeInOutBack",
                                          "easeInElastic",
                                          "easeOutElastic",
                                          "easeInOutElastic",
                                          "easeInBounce",
                                          "easeOutBounce",
                                          "easeInOutBounce"};

        QStringList displayNames = {
            "Linear/Constant",

            "Quad In",         "Quad Out",     "Quad In Out",    "Cubic In",
            "Cubic Out",       "Cubic In Out", "Quart In",       "Quart Out",
            "Quart In Out",    "Quint In",     "Quint Out",      "Quint In Out",
            "Sine In",         "Sine Out",     "Sine In Out",    "Expo In",
            "Expo Out",        "Expo In Out",  "Circ In",        "Circ Out",
            "Circ In Out",     "Back In",      "Back Out",       "Back In Out",
            "Elastic In",      "Elastic Out",  "Elastic In Out", "Bounce In",
            "Bounce Out",      "Bounce In Out"};
        auto input = new QComboBox(this);

        for (size_t i = 0; i < names.size(); i++) {
            QPixmap pixmap(24, 24);
            pixmap.fill(Qt::transparent);
            QPainter pixmapPainter(&pixmap);
            pixmapPainter.setPen(Qt::NoPen);
            for (int x = 0; x < pixmap.width(); x++) {
                double xValue = functions[i]((double)x / pixmap.width());
                if (xValue > 1 || xValue < 0) {
                    pixmapPainter.setBrush(QColor(255, 100, 100));
                } else {
                    pixmapPainter.setBrush(palette().text());
                }
                int xHeight = xValue * pixmap.height();
                pixmapPainter.drawRect(x, pixmap.height() - xHeight, 1,
                                       xHeight);
            }
            pixmapPainter.end();
            QIcon previewIcon(pixmap);

            input->addItem(previewIcon, displayNames[i]);
        }

        auto value = variant.get<Easing>();
        input->setCurrentIndex(
            std::find(names.begin(), names.end(), value.easingCurve) -
            names.begin());
        connect(input, &QComboBox::currentIndexChanged, this,
                [this, names](int index) { set(Easing{names[index]}); });
        widget = input;
    } else if (variantType == VariantTypeEnum::Brush) {
        auto input = new BrushInput(this);
        Brush value = variant.get<Brush>();
        input->setValue(value);
        connect(input, &BrushInput::valueChanged, this,
                [this](Brush newValue) { set(newValue); });
        widget = input;
    }

    if (widget) {
        lay->addWidget(widget);
    }
}

template <typename T> void PropertyEdit::set(T newValue) {
    auto property = (Property<T> *)this->property;
    property->set(newValue, {scene->currentFrame});
}
