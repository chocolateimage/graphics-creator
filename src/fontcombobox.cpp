#include "fontcombobox.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <fontconfig/fontconfig.h>

FontComboBox::FontComboBox(QWidget *parent) : QComboBox(parent) {
    addItems({"Hello", "World"});
};

void FontComboBox::showPopup() {
    qInfo() << "show";

    if (!popupWindow) {
        // TODO: reload properly
        FcConfigUptoDate(nullptr);
        FcPattern *pattern = FcPatternCreate();
        FcObjectSet *objectSet = FcObjectSetBuild(
            FC_FILE, FC_INDEX, FC_FAMILY, FC_WEIGHT, FC_SLANT, FC_STYLE, NULL);
        FcFontSet *fontSet = FcFontList(nullptr, pattern, objectSet);
        std::map<std::string, std::shared_ptr<FontPopupGroup>> fontGroups;
        for (int i = 0; i < fontSet->nfont; i++) {
            FcPattern *font = fontSet->fonts[i];
            FcChar8 *rawFamily;
            FcChar8 *rawFileName;
            FcChar8 *rawStyle;
            int fontIndex;
            int weight;
            int slant;
            FcPatternGetString(font, FC_FILE, 0, &rawFileName);
            FcPatternGetString(font, FC_FAMILY, 0, &rawFamily);
            FcPatternGetString(font, FC_STYLE, 0, &rawStyle);
            FcPatternGetInteger(font, FC_INDEX, 0, &fontIndex);
            FcPatternGetInteger(font, FC_WEIGHT, 0, &weight);
            FcPatternGetInteger(font, FC_SLANT, 0, &slant);
            std::string family((char *)rawFamily);
            std::string fileName((char *)rawFileName);
            std::string style((char *)rawStyle);

            auto it = fontGroups.find(family);
            std::shared_ptr<FontPopupGroup> group;
            if (it == fontGroups.end()) {
                group = std::make_shared<FontPopupGroup>();
                group->family = family;
                group->path = fileName;
                fontGroups.emplace(family, group);
            } else {
                group = it->second;
            }

            FontPopupStyle fontStyle;
            fontStyle.index = fontIndex;
            fontStyle.weight = weight;
            fontStyle.slant = slant;
            group->styles.push_back(fontStyle);
        }
        FcFontSetDestroy(fontSet);
        FcObjectSetDestroy(objectSet);
        FcPatternDestroy(pattern);

        std::vector<std::shared_ptr<FontPopupGroup>> fontGroupsList;
        for (auto group : fontGroups) {
            std::sort(group.second->styles.begin(), group.second->styles.end(),
                      [](const FontPopupStyle &a, const FontPopupStyle &b) {
                          return (a.weight + a.slant) < (b.weight + b.slant);
                      });
            fontGroupsList.push_back(group.second);
        }
        fontGroups.clear();
        std::sort(fontGroupsList.begin(), fontGroupsList.end(),
                  [](std::shared_ptr<FontPopupGroup> a,
                     std::shared_ptr<FontPopupGroup> b) {
                      return a->family < b->family;
                  });

        popupWindow = new FontComboBoxPopup();
        popupWindow->setFrameShape(QFrame::Shape::StyledPanel);
        popupWindow->setFrameShadow(QFrame::Shadow::Sunken);
        popupWindow->setProperty("_breeze_force_frame", true);
        popupWindow->setWindowFlag(Qt::WindowType::Popup);
        popupWindow->setWindowFlag(Qt::WindowType::FramelessWindowHint);

        auto lay1 = new QVBoxLayout(popupWindow);
        lay1->setContentsMargins(0, 0, 0, 0);

        auto lay2 = new QHBoxLayout();
        lay2->setContentsMargins(0, 0, 0, 0);
        lay1->addLayout(lay2);

        popupWindow->searchInput = new QLineEdit(popupWindow);
        popupWindow->searchInput->setPlaceholderText("Search fonts…");
        lay2->addWidget(popupWindow->searchInput);

        auto addButton = new QPushButton(popupWindow);
        addButton->setText("Add…");
        addButton->setIcon(QIcon::fromTheme("list-add"));
        lay2->addWidget(addButton);

        auto scroll = new QScrollArea(popupWindow);
        scroll->setFrameShape(QFrame::Shape::NoFrame);
        auto content = new QWidget();
        scroll->setWidget(content);
        scroll->setWidgetResizable(true);
        auto lay = new QVBoxLayout(content);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);
        for (auto group : fontGroupsList) {
            auto button = new FontPopupFontWidget(content);
            new QVBoxLayout(button);
            button->setFlat(true);
            auto buttonText =
                new QLabel(QString::fromStdString(group->family), button);
            button->layout()->addWidget(buttonText);
            buttonText->setAlignment(Qt::AlignLeft);
            lay->addWidget(button);
        }

        lay1->addWidget(scroll);
    }

    popupWindow->move(mapToGlobal(rect().bottomLeft()));
    popupWindow->resize(width(), 500);
    popupWindow->show();
    popupWindow->searchInput->setFocus();
    popupWindow->searchInput->selectAll();
}

void FontComboBox::hidePopup() {
    qInfo() << "hide";
    if (popupWindow) {
        popupWindow->close();
        delete popupWindow;
    }
}

FontPopupFontWidget::FontPopupFontWidget(QWidget *parent)
    : QPushButton(parent) {};
