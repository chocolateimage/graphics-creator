#include "presets.hpp"
#include "animatable/element/text_element.hpp"
#include "gui.hpp"
#include <QMimeData>
#include <QVBoxLayout>

bool TextPreset::canApply(Element *element) {
    return dynamic_cast<TextElement *>(element) != nullptr;
}

void TextPreset::apply(Element *element) { applyFunc((TextElement *)element); }

QMimeData *
PresetsListWidget::mimeData(const QList<QListWidgetItem *> &items) const {
    QMimeData *mimeData = new QMimeData();
    if (!items.isEmpty()) {
        QListWidgetItem *item = items.first();
        mimeData->setData(PRESET_MIME_TYPE,
                          item->data(Qt::UserRole).toString().toUtf8());
    }
    return mimeData;
}

PresetsWindow::PresetsWindow(Scene *scene, NewMainWindow *mainWindow)
    : scene(scene), mainWindow(mainWindow) {
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    PresetsListWidget *listWidget = new PresetsListWidget();
    listWidget->setViewMode(QListWidget::IconMode);
    listWidget->setMovement(QListWidget::Static);
    listWidget->setUniformItemSizes(true);
    listWidget->setWrapping(true);
    listWidget->setWordWrap(true);
    listWidget->setResizeMode(QListWidget::Adjust);
    listWidget->setSpacing(4);
    listWidget->setDragEnabled(true);
    listWidget->setIconSize({75, 42});
    connect(listWidget, &QListWidget::itemDoubleClicked, this,
            &PresetsWindow::doubleClicked);
    lay->addWidget(listWidget);

    presets.append(new TextPreset(
        "Text Fading In Down", "fading-in-down", [](Element *element) {
            TextElement *textElement = (TextElement *)element;
            TextAnimator *newAnimator = new TextAnimator(textElement);
            TextAnimatorSelector *selector =
                new TextAnimatorSelector(newAnimator);
            newAnimator->selectors.append(selector);
            textElement->textAnimators.append(newAnimator);
            emit textElement->effectListUpdated();
        }));

    for (Preset *preset : presets) {
        QIcon icon(
            QPixmap(mainWindow->dataPath + "/presets/" + preset->id + ".png"));
        QListWidgetItem *item = new QListWidgetItem(icon, preset->displayName);
        item->setSizeHint(QSize(90, 90));
        item->setToolTip(preset->displayName);
        item->setData(Qt::UserRole, QString::number((uint64_t)(preset)));
        listWidget->addItem(item);
    }
}

void PresetsWindow::doubleClicked(QListWidgetItem *item) {
    uint64_t address = item->data(Qt::UserRole).toULongLong();
    Preset *preset = (Preset *)address;

    for (Element *element : scene->selectedElements) {
        if (preset->canApply(element)) {
            preset->apply(element);
        }
    }

    if (scene->selectedElements.isEmpty()) {
        if (preset->canApply(nullptr)) {
            preset->apply(nullptr);
        }
    }
}
