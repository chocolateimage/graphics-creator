#pragma once
#include <QDialog>
#include <QPushButton>

class PluginManagerDialog : public QDialog {
    Q_OBJECT
  public:
    PluginManagerDialog(QWidget *parent);

    void openFolder();
};
