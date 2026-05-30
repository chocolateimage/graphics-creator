#pragma once

#include "variant.hpp"
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <freetype/freetype.h>

class FontComboBoxPopup;
class FontPopupFontWidget;

struct FontPopupStyle {
    int index;
    int weight;
    int slant;
    std::string path;
    std::string displayName;
};

class FontPopupGroup {
  public:
    std::string family;
    std::vector<FontPopupStyle> styles;

    FontPopupStyle &getDefaultStyle();
};

class FontComboBox : public QComboBox {
    Q_OBJECT
  public:
    explicit FontComboBox(QWidget *parent = nullptr);
    ~FontComboBox();
    void showPopup() override;
    void hidePopup() override;
    void selectStyle(std::shared_ptr<FontPopupGroup> group,
                     FontPopupStyle &style);

    void setFontValue(Font font);
    Font fontValue();

    FontComboBoxPopup *popupWindow{nullptr};

    FT_Library ftLibrary{nullptr};
};

class FontComboBoxPopup : public QFrame {
    Q_OBJECT
  public:
    explicit FontComboBoxPopup();

    FontComboBox *comboBox;
    int amountOfFonts;
    QLineEdit *searchInput;
    QScrollArea *scrollArea;
    QList<FontPopupFontWidget *> buttons;
    void closeAllButtons();

    void searchChanged(const QString &value);
};

class FontPopupFontWidget : public QPushButton {
    Q_OBJECT
  public:
    explicit FontPopupFontWidget(FT_Library ftLibrary,
                                 std::shared_ptr<FontPopupGroup> group,
                                 QWidget *parent = nullptr);
    std::shared_ptr<FontPopupGroup> group;

    FontComboBoxPopup *comboBoxPopup;
    QLabel *buttonText;

    void closeButton();
    void buttonClicked();
    QGroupBox *styleGroupBox;

    FT_Library ftLibrary;

    QSize sizeHint() const override { return QWidget::sizeHint(); }
    QSize minimumSizeHint() const override { return QWidget::minimumSize(); }
};

class FontPopupFontPreview : public QLabel {
    Q_OBJECT
  public:
    explicit FontPopupFontPreview(FT_Library ftLibrary,
                                  std::shared_ptr<FontPopupGroup> group,
                                  FontPopupStyle &style,
                                  QWidget *parent = nullptr);

    bool created{false};
    void createInside();

    FT_Library ftLibrary;
    std::shared_ptr<FontPopupGroup> group;
    FontPopupStyle &style;

  protected:
    bool event(QEvent *e) override;
};
