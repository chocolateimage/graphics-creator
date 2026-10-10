#include "presets_window.hpp"
#include "gui/gui.hpp"
#include <QMimeData>
#include <QVBoxLayout>

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

    listWidget = new PresetsListWidget();
    listWidget->setSupportedDragActions({Qt::DropAction::CopyAction});
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

    reloadPresets();
}

void PresetsWindow::reloadPresets() {
    listWidget->clear();
    for (Preset *preset : presetList) {
        QImage img(mainWindow->dataPath + "/presets/" + preset->id + ".png");
        if (palette().text().color().lightnessF() < 0.5) {
            img.invertPixels();
        }
        QIcon icon(QPixmap::fromImage(img));
        QListWidgetItem *item = new QListWidgetItem(icon, preset->displayName);
        item->setSizeHint(QSize(90, 90));
        item->setToolTip(preset->displayName);
        item->setData(Qt::UserRole, QString::number((uint64_t)(preset)));
        listWidget->addItem(item);
    }
}

bool PresetsWindow::event(QEvent *event) {
    if (event->type() == QEvent::PaletteChange) {
        reloadPresets();
    }

    return QWidget::event(event);
}

void PresetsWindow::doubleClicked(QListWidgetItem *item) {
    uint64_t address = item->data(Qt::UserRole).toULongLong();
    Preset *preset = (Preset *)address;

    for (Element *element : scene->selectedElements) {
        if (preset->canApply(mainWindow, element)) {
            preset->apply(mainWindow, element);
        }
    }

    if (scene->selectedElements.isEmpty()) {
        if (preset->canApply(mainWindow, nullptr)) {
            preset->apply(mainWindow, nullptr);
        }
    }
}
