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

    for (Preset *preset : presetList) {
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
