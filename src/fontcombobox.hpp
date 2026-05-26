#pragma once

#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

class FontComboBoxPopup;

class FontComboBox : public QComboBox {
    Q_OBJECT
  public:
    explicit FontComboBox(QWidget *parent = nullptr);
    void showPopup() override;
    void hidePopup() override;

    FontComboBoxPopup *popupWindow{nullptr};

  protected:
};

class FontComboBoxPopup : public QFrame {
    Q_OBJECT
  public:
    QLineEdit *searchInput;
};

struct FontPopupStyle {
    int index;
    int weight;
    int slant;
};

struct FontPopupGroup {
    std::string path;
    std::string family;
    std::vector<FontPopupStyle> styles;
};

class FontPopupFontWidget : public QPushButton {
    Q_OBJECT
  public:
    explicit FontPopupFontWidget(QWidget *parent = nullptr);
};
