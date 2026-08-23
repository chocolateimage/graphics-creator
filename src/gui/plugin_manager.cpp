#include "plugin_manager.hpp"
#include "plugin.hpp"
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include <QUrl>

PluginManagerDialog::PluginManagerDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Plugins");
    resize(600, 400);

    QVBoxLayout *lay = new QVBoxLayout(this);
    QTreeWidget *treeWidget = new QTreeWidget();
    treeWidget->setColumnCount(3);
    treeWidget->setHeaderLabels({"Name", "Version", "Path"});
    treeWidget->setColumnWidth(0, 150);
    treeWidget->setColumnWidth(1, 50);
    treeWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    treeWidget->setRootIsDecorated(false);
    treeWidget->setSortingEnabled(true);
    treeWidget->header()->setSectionsMovable(false);
    for (auto plugin : pluginManager->loadedPlugins) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, plugin->name);
        item->setText(1, plugin->version);
        item->setText(2, plugin->path);
        treeWidget->addTopLevelItem(item);
    }
    lay->addWidget(treeWidget);

    QHBoxLayout *lay2 = new QHBoxLayout();
    lay->addLayout(lay2);
    lay2->setContentsMargins(0, 0, 0, 0);

    lay2->addStretch();

    QPushButton *folderButton = new QPushButton("Open plugins folder");
    connect(folderButton, &QPushButton::clicked, this,
            &PluginManagerDialog::openFolder);
    lay2->addWidget(folderButton);
}

void PluginManagerDialog::openFolder() {
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(pluginManager->defaultPluginPath));
}
