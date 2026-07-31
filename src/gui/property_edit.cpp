#include "property_edit.hpp"
#include "brush_input.hpp"
#include "draggable_spinbox.hpp"
#include "fontcombobox.hpp"
#include "line.hpp"
#include <KColorButton>
#include <KIconColors>
#include <KIconLoader>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QPlainTextEdit>
#include <QSlider>
#include <QToolButton>

PropertyToggleAnimationButton::PropertyToggleAnimationButton(
    Scene *scene, PropertyBase *property, QWidget *parent)
    : QPushButton(parent), scene(scene), property(property) {
    setFlat(true);
    animationUpdated(property);

    connect(this, &QPushButton::clicked, this,
            &PropertyToggleAnimationButton::toggleAnimationClicked);
    connect(property->animatable, &Animatable::propertyIsAnimatingUpdated, this,
            &PropertyToggleAnimationButton::animationUpdated);
}

void PropertyToggleAnimationButton::animationUpdated(
    PropertyBase *updatedProperty) {
    if (updatedProperty != property)
        return;

    if (property->isAnimating) {
        KIconColors colors;
        colors.setText(palette().accent().color());
        setIcon(KDE::icon("keyframe", colors));
        setToolTip("Animation enabled");
    } else {
        setIcon(QIcon::fromTheme("keyframe-disable"));
        setToolTip("Animation disabled");
    }
}

void PropertyToggleAnimationButton::toggleAnimationClicked() {
    property->toggleAnimating({scene->currentFrame});
}

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
        // TODO: undo
        auto input = new QPlainTextEdit(this);
        input->setProperty("_breeze_force_frame", true);

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
            // TODO: undo
            auto input = new QSlider(this);
            input->setOrientation(Qt::Horizontal);
            input->setMinimum(min);
            input->setMaximum(max);
            input->setValue(variant.get<int>());
            connect(input, &QSlider::valueChanged, this,
                    [this](int value) { set(value); });
            widget = input;
        } else if (!propertyTyped->enumList.empty()) {
            auto input = new QComboBox(this);
            for (const auto &item : propertyTyped->enumList) {
                input->addItem(QString::fromStdString(item));
            }
            input->setCurrentIndex(variant.get<int>());

            connect(input, &QComboBox::currentIndexChanged, this,
                    [this](int value) {
                        beginEditing();
                        set(value);
                        finishEditing();
                    });
            widget = input;
        } else {
            auto input = new DraggableSpinBox(this);
            input->setMinimum(min);
            input->setMaximum(max);
            input->setSuffix(QString::fromStdString(propertyTyped->suffix));
            input->setValue(variant.get<int>());
            connect(input, &DraggableSpinBox::editingFinished, this,
                    [this]() { finishEditing(); });
            connect(input, &DraggableSpinBox::valueChanged, this,
                    [this](int value) {
                        beginEditing();
                        set(value);
                    });
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
        input->setSuffix(QString::fromStdString(propertyTyped->suffix));
        input->setValue(variant.get<double>());
        connect(input, &DraggableDoubleSpinBox::editingFinished, this,
                [this]() { finishEditing(); });
        connect(input, &DraggableDoubleSpinBox::valueChanged, this,
                [this](double value) {
                    beginEditing();
                    set(value);
                });
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
                    beginEditing();
                    set((Vector2DInt){value, inputY->value()});
                });
        connect(inputY, &DraggableSpinBox::valueChanged, this,
                [this, inputX](int value) {
                    beginEditing();
                    set((Vector2DInt){inputX->value(), value});
                });
        connect(inputX, &DraggableSpinBox::editingFinished, this,
                [this]() { finishEditing(); });
        connect(inputY, &DraggableSpinBox::editingFinished, this,
                [this]() { finishEditing(); });
        // connect(pickButton, &QToolButton::clicked, this, [this,
        // optionLabel]() {
        //     previewWidget->beginPicking(QString::fromStdString(optionId),
        //                                 "Pick \"" + optionLabel + "\"",
        //                                 ImageViewer::PickType::Point);
        // });
    } else if (variantType == VariantTypeEnum::Vector2DFloat) {
        widget = new QWidget(this);
        auto layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        auto value = variant.get<Vector2DFloat>();

        auto inputX = new DraggableDoubleSpinBox(this);
        inputX->setMinimum(min);
        inputX->setMaximum(max);
        inputX->setValue(value.x);

        auto inputY = new DraggableDoubleSpinBox(this);
        inputY->setMinimum(min);
        inputY->setMaximum(max);
        inputY->setValue(value.y);

        layout->addWidget(inputX);
        layout->addWidget(inputY);

        connect(inputX, &DraggableDoubleSpinBox::valueChanged, this,
                [this, inputY](double value) {
                    beginEditing();
                    set((Vector2DFloat){(float)value, (float)inputY->value()});
                });
        connect(inputY, &DraggableDoubleSpinBox::valueChanged, this,
                [this, inputX](double value) {
                    beginEditing();
                    set((Vector2DFloat){(float)inputX->value(), (float)value});
                });
        connect(inputX, &DraggableDoubleSpinBox::editingFinished, this,
                [this]() { finishEditing(); });
        connect(inputY, &DraggableDoubleSpinBox::editingFinished, this,
                [this]() { finishEditing(); });
    } else if (variantType == VariantTypeEnum::Color) {
        auto colorButton = new KColorButton(this);
        colorButton->setAlphaChannelEnabled(true);
        auto value = variant.get<Color>();
        colorButton->setColor(QColor(value.r, value.g, value.b, value.a));
        connect(colorButton, &KColorButton::changed, this,
                [this](const QColor &newColor) {
                    beginEditing();
                    set((Color){newColor.red(), newColor.green(),
                                newColor.blue(), newColor.alpha()});
                    finishEditing();
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
                [this, fontWidget]() {
                    beginEditing();
                    set(fontWidget->fontValue());
                    finishEditing();
                });

        widget = fontWidget;
    } else if (variantType == VariantTypeEnum::Bool) {
        auto input = new QCheckBox(this);
        input->setChecked(variant.get<bool>());
        connect(input, &QCheckBox::checkStateChanged, this,
                [this](Qt::CheckState checkState) {
                    beginEditing();
                    set(checkState == Qt::CheckState::Checked);
                    finishEditing();
                });
        widget = input;
    } else if (variantType == VariantTypeEnum::Easing) {
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
            auto function = Easing{names[i]}.toFunction();
            for (int x = 0; x < pixmap.width(); x++) {
                double xValue = function((double)x / pixmap.width());
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
                [this, names](int index) {
                    beginEditing();
                    set(Easing{names[index]});
                    finishEditing();
                });
        widget = input;
    } else if (variantType == VariantTypeEnum::Brush) {
        auto input = new BrushInput(this);
        Brush value = variant.get<Brush>();
        input->setValue(value);
        connect(input, &BrushInput::valueChanged, this, [this](Brush newValue) {
            beginEditing();
            set(newValue);
        });
        connect(input, &BrushInput::editingFinished, this,
                [this]() { finishEditing(); });
        widget = input;
    } else if (variantType == VariantTypeEnum::TextSpans) {
        auto button = new QPushButton(this);
        button->setText("Edit");
        button->setIcon(QIcon::fromTheme("document-edit"));
        connect(button, &QPushButton::clicked, this, [this]() {
            ((Element *)this->property->animatable)->setEditMode(true);
        });
        widget = button;
    } else if (variantType == VariantTypeEnum::ElementSelection) {
        widget = new QWidget(this);
        auto layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        auto value = variant.get<ElementSelection>();

        auto inputElement = new QComboBox(this);
        int index = 0;
        int selectedIndex = -1;
        for (auto element : scene->elements) {
            inputElement->addItem(element->objectName(), element->id);
            if (element->id == value.elementId) {
                selectedIndex = index;
            }
            index++;
        }

        inputElement->setCurrentIndex(selectedIndex);

        auto inputType = new QComboBox(this);
        inputType->addItem("Source");
        inputType->addItem("Final");
        inputType->setCurrentIndex(value.frameType);

        layout->addWidget(inputElement);
        layout->addWidget(inputType);

        auto inputUpdated = [this, inputElement, inputType]() {
            beginEditing();
            ElementSelection elementSelection;
            elementSelection.elementId = inputElement->currentData().toString();
            elementSelection.frameType =
                (ElementSelection::FrameType)inputType->currentIndex();
            set(elementSelection);
            finishEditing();
        };

        connect(inputElement, &QComboBox::currentIndexChanged, this,
                inputUpdated);
        connect(inputType, &QComboBox::currentIndexChanged, this, inputUpdated);
    }

    if (widget) {
        lay->addWidget(widget);
    }
}

template <typename T> void PropertyEdit::set(T newValue) {
    auto property = (Property<T> *)this->property;
    property->set(newValue, {scene->currentFrame});
}

class PropertyEditCommand : public QUndoCommand {
  public:
    PropertyEditCommand(Scene *scene, PropertyBase *property,
                        QJsonObject oldObj, QJsonObject newObj)
        : scene(scene), property(property), oldObj(oldObj), newObj(newObj) {
        setText("Change " + property->getDisplayName());
    }
    ~PropertyEditCommand() {}

    Scene *scene;
    PropertyBase *property;
    QJsonObject oldObj;
    QJsonObject newObj;
    bool didDo{false};

    void undo() override {
        property->deserialize(oldObj);
        property->animatable->_propertyUpdated(property);
        scene->selectElements(scene->selectedElements); // hack
    }
    void redo() override {
        if (!didDo) {
            didDo = true;
            return;
        }
        property->deserialize(newObj);
        property->animatable->_propertyUpdated(property);
        scene->selectElements(scene->selectedElements); // hack
    }
};

void PropertyEdit::beginEditing() {
    if (isEditing) {
        return;
    }
    isEditing = true;
    savedState = property->serialize();
}

void PropertyEdit::finishEditing() {
    if (!isEditing) {
        return;
    }
    isEditing = false;
    PropertyEditCommand *command = new PropertyEditCommand(
        scene, property, std::move(savedState), property->serialize());
    scene->undoStack->push(command);
}
