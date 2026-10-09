#include "presets.hpp"
#include "animatable/element/text_element.hpp"
#include "gui/gui.hpp"

QList<Preset *> presetList = {};

void initPresets() {
    presetList.append(new TextPreset(
        "Text Fading In Down", "fading-in-down", [](Element *element) {
            TextElement *textElement = (TextElement *)element;
            TextAnimator *newAnimator = new TextAnimator(textElement);
            TextAnimatorSelector *selector =
                new TextAnimatorSelector(newAnimator);

            int offset = element->scene->currentFrame;
            int start = offset;
            int end = offset + element->scene->frameRate * 0.5;
            selector->offset.toggleAnimating({start});
            selector->offset.set(-100, {start});
            selector->offset.set(100, {end});
            selector->easing.set(Easing{"easeInCubic"}, {0});
            newAnimator->opacity.set(0, {0});

            int fontSize = 100;
            auto spans = textElement->text.get({0}).spans;
            if (!spans.isEmpty()) {
                fontSize = spans.first().fontSize;
            }

            newAnimator->opacity.set(0, {0});
            newAnimator->y.set(-fontSize, {0});

            newAnimator->selectors.append(selector);
            textElement->textAnimators.append(newAnimator);
            emit textElement->effectListUpdated();
        }));
}

bool TextPreset::canApply(NewMainWindow *mainWindow, Element *element) {
    return element == nullptr ||
           dynamic_cast<TextElement *>(element) != nullptr;
}

Element *TextPreset::apply(NewMainWindow *mainWindow, Element *element) {
    if (element == nullptr) {
        TextElement *textElement = new TextElement();
        textElement->x.set(0, {0});
        textElement->y.set(0, {0});
        textElement->w.set(0, {0});
        textElement->h.set(0, {0});
        textElement->setObjectName(displayName);
        mainWindow->addElementUndoable(textElement);
        element = textElement;
    }
    applyFunc((TextElement *)element);
    return element;
}
