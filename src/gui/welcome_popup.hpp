#pragma once
#include <QCheckBox>
#include <QDialog>

class NewMainWindow;

class WelcomePopup : public QDialog {
    Q_OBJECT
  public:
    explicit WelcomePopup(NewMainWindow *parent = nullptr);

    void acceptedSlot();

    QCheckBox *updateCheck;
};
