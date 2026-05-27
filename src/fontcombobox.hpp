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
    std::string path;
};

class FontPopupGroup {
  public:
    std::string family;
    std::vector<FontPopupStyle> styles;

    FontPopupStyle &getDefaultStyle();
};

class FontPopupFontWidget : public QPushButton {
    Q_OBJECT
  public:
    explicit FontPopupFontWidget(std::shared_ptr<FontPopupGroup> group,
                                 QWidget *parent = nullptr);
    std::shared_ptr<FontPopupGroup> group;

    QSize sizeHint() const override { return QWidget::sizeHint(); }
    QSize minimumSizeHint() const override { return QWidget::minimumSize(); }
};
