#pragma once

#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

class FontComboBox : public QComboBox {
    Q_OBJECT
  public:
    explicit FontComboBox(QWidget *parent = nullptr);
    void showPopup() override;
    void hidePopup() override;

    QFrame *popupWindow{nullptr};

  protected:
};

class FontComboBoxPopup : public QFrame {
    Q_OBJECT
  protected:
    void paintEvent(QPaintEvent *e) override;
};
