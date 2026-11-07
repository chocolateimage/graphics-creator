#pragma once
#include <QDialog>

class QCheckBox;
class NewMainWindow;

class WelcomePopup : public QDialog {
    Q_OBJECT
  public:
    explicit WelcomePopup(NewMainWindow *parent = nullptr);

    void acceptedSlot();

    QCheckBox *updateCheck;
};
