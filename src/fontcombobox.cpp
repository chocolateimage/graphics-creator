#include "fontcombobox.hpp"
#include "math.hpp"
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <fontconfig/fontconfig.h>
#include <freetype/ftglyph.h>
#include <ft2build.h>
#include <string>
#include FT_FREETYPE_H
#include <QFontDatabase>
#include <QTimer>
#include <hb-ft.h>
#include <hb.h>

FontComboBox::FontComboBox(QWidget *parent) : QComboBox(parent) {
    addItems({"Hello", "World"});
};

void FontComboBox::showPopup() {
    qInfo() << "show";

    if (!ftLibrary) {
        FT_Init_FreeType(&ftLibrary);
    }

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
                fontGroups.emplace(family, group);
            } else {
                group = it->second;
            }

            FontPopupStyle fontStyle;
            fontStyle.index = fontIndex;
            fontStyle.weight = weight;
            fontStyle.slant = slant;
            fontStyle.path = fileName;
            fontStyle.displayName = style;
            group->styles.push_back(fontStyle);
        }
        FcFontSetDestroy(fontSet);
        FcObjectSetDestroy(objectSet);
        FcPatternDestroy(pattern);

        std::vector<std::shared_ptr<FontPopupGroup>> fontGroupsList;
        for (auto group : fontGroups) {
            std::sort(group.second->styles.begin(), group.second->styles.end(),
                      [](const FontPopupStyle &a, const FontPopupStyle &b) {
                          return (a.weight * 10 + a.slant) <
                                 (b.weight * 10 + b.slant);
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
            auto button = new FontPopupFontWidget(ftLibrary, group, content);
            lay->addWidget(button);
        }

        lay->addStretch();

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
        popupWindow = nullptr;
    }
}

FontPopupStyle &FontPopupGroup::getDefaultStyle() {
    for (auto &style : styles) {
        if (style.weight == FC_WEIGHT_REGULAR &&
            style.slant == FC_SLANT_ROMAN) {
            return style;
        }
    }
    return styles[0];
}

FontComboBox::~FontComboBox() {
    if (ftLibrary) {
        FT_Done_FreeType(ftLibrary);
        ftLibrary = nullptr;
    }
}

FontPopupFontWidget::FontPopupFontWidget(FT_Library ftLibrary,
                                         std::shared_ptr<FontPopupGroup> group,
                                         QWidget *parent)
    : QPushButton(parent), group(group), ftLibrary(ftLibrary) {

    auto lay = new QVBoxLayout(this);
    setFlat(true);

    auto familyText = new QLabel(this);
    familyText->setEnabled(false);
    lay->addWidget(familyText);

    QString family = QString::fromStdString(group->family);
    setToolTip(family);
    familyText->setText(family);

    buttonText = new FontPopupFontPreview(ftLibrary, group,
                                          group->getDefaultStyle(), this);
    setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Fixed);

    lay->addWidget(buttonText);

    lay->addSpacing(8);

    styleGroupBox = new QGroupBox(this);
    styleGroupBox->setContentsMargins(0, 0, 0, 0);
    styleGroupBox->setTitle("Styles");
    styleGroupBox->hide();

    auto tabLayout = new QVBoxLayout(styleGroupBox);
    tabLayout->setSpacing(0);
    tabLayout->setContentsMargins(0, 0, 0, 0);

    for (auto &style : group->styles) {
        auto btn = new QPushButton(styleGroupBox);
        btn->setFlat(true);
        auto btnLay = new QVBoxLayout(btn);
        btnLay->setSizeConstraint(QLayout::SetMinimumSize);

        auto lbl = new QLabel(QString::fromStdString(style.displayName), btn);
        btnLay->addWidget(lbl);

        auto preview = new FontPopupFontPreview(ftLibrary, group, style, btn);
        btnLay->addWidget(preview);

        tabLayout->addWidget(btn);
    }

    lay->addWidget(styleGroupBox);

    connect(this, &QPushButton::clicked, this,
            &FontPopupFontWidget::buttonClicked);
}

void FontPopupFontWidget::buttonClicked() {
    setFlat(false);
    styleGroupBox->show();
}

FontPopupFontPreview::FontPopupFontPreview(
    FT_Library ftLibrary, std::shared_ptr<FontPopupGroup> group,
    FontPopupStyle &style, QWidget *parent)
    : QLabel(parent), ftLibrary(ftLibrary), group(group), style(style) {}

void FontPopupFontPreview::createInside() {
    if (created)
        return;

    created = true;

    FT_Face ftFace;

    FT_New_Face(ftLibrary, style.path.c_str(), style.index, &ftFace);

    float scale = devicePixelRatio();

    FT_Set_Pixel_Sizes(ftFace, 0, 24 * scale);

    hb_font_t *hbFont = hb_ft_font_create(ftFace, nullptr);

    hb_buffer_t *hbBuffer = hb_buffer_create();
    QString text = QString::fromStdString(group->family);

    auto systems =
        QFontDatabase::writingSystems(QString::fromStdString(group->family));

    while (FT_Get_Char_Index(ftFace, text.toStdU32String()[0]) == 0 &&
           !systems.isEmpty()) {
        auto sample = QFontDatabase::writingSystemSample(systems.takeFirst());
        if (!sample.isEmpty()) {
            text = sample;
        }
    }

    if (FT_Get_Char_Index(ftFace, text.toStdU32String()[0]) == 0) {
        std::u32string u32;
        FT_UInt index;
        FT_ULong character = FT_Get_First_Char(ftFace, &index);
        while (u32.size() < 32) {
            character = FT_Get_Next_Char(ftFace, character, &index);
            if (!index)
                break;
            u32 = U" " + u32;
            u32[0] = character;
        }
        text = QString::fromStdU32String(u32);
    }

    hb_buffer_add_utf8(hbBuffer, qPrintable(text), -1, 0, -1);
    hb_buffer_guess_segment_properties(hbBuffer);
    hb_shape(hbFont, hbBuffer, nullptr, 0);

    //

    uint32_t glyphCount;
    hb_glyph_info_t *glyphInfo =
        hb_buffer_get_glyph_infos(hbBuffer, &glyphCount);
    hb_glyph_position_t *glyphPositions =
        hb_buffer_get_glyph_positions(hbBuffer, &glyphCount);

    std::unordered_map<hb_codepoint_t, FT_BitmapGlyph> glyphs;

    int minX = INT_MAX;
    int minY = INT_MAX;
    int maxX = INT_MIN;
    int maxY = INT_MIN;
    int penX = 0;
    int penY = 0;
    unsigned int textImageWidth;
    unsigned int textImageHeight;

    for (uint32_t i = 0; i < glyphCount; i++) {
        auto codepoint = glyphInfo[i].codepoint;
        auto it = glyphs.find(codepoint);
        FT_BitmapGlyph glyph;
        if (it == glyphs.end()) {
            FT_Glyph _glyph;
            FT_Load_Glyph(ftFace, codepoint,
                          FT_LOAD_RENDER | FT_LOAD_TARGET_LCD);
            FT_Get_Glyph(ftFace->glyph, &_glyph);

            glyph = (FT_BitmapGlyph)_glyph;
            glyphs[codepoint] = glyph;
        } else {
            glyph = it->second;
        }

        auto glyphPos = glyphPositions[i];
        int xOffset = glyphPos.x_offset >> 6;
        int yOffset = glyphPos.y_offset >> 6;
        int xAdvance = glyphPos.x_advance >> 6;
        int yAdvance = glyphPos.y_advance >> 6;

        int drawX = (penX + xOffset + glyph->left);
        int drawY = (penY + yOffset - glyph->top);

        minX = std::min(minX, drawX);
        minY = std::min(minY, drawY);
        maxX = std::max(maxX, (int)glyph->bitmap.width + drawX);
        maxY = std::max(maxY, (int)glyph->bitmap.rows + drawY);

        penX += xAdvance;
        penY += yAdvance;
    }

    textImageWidth = maxX - minX;
    textImageHeight = maxY - minY;

    uint8_t *textImage = new uint8_t[textImageWidth * textImageHeight * 4];
    memset(textImage, 0, textImageWidth * textImageHeight * 4);

    QColor textColor = palette().text().color();
    int textColorParts[] = {textColor.blue(), textColor.green(),
                            textColor.red()};

    penX = 0;
    penY = 0;
    for (uint32_t i = 0; i < glyphCount; i++) {
        auto glyphPos = glyphPositions[i];
        auto codepoint = glyphInfo[i].codepoint;
        int xOffset = glyphPos.x_offset >> 6;
        int yOffset = glyphPos.y_offset >> 6;
        int xAdvance = glyphPos.x_advance >> 6;
        int yAdvance = glyphPos.y_advance >> 6;
        auto glyph = glyphs[glyphInfo[i].codepoint];

        int drawX = (penX + xOffset + glyph->left) - minX;
        int drawY = (penY + yOffset - glyph->top) - minY;

        for (unsigned int y = 0; y < glyph->bitmap.rows; y++) {
            for (unsigned int x = 0; x < glyph->bitmap.width / 3; x++) {
                int targetX = drawX + x;
                int targetY = drawY + y;
                int targetIndex = (targetY * textImageWidth * 4 + targetX * 4);
                for (int subPixel = 0; subPixel < 3; subPixel++) {
                    int index = targetIndex + subPixel;

                    uint8_t value = glyph->bitmap.buffer[(
                        y * glyph->bitmap.pitch + (x * 3) + (2 - subPixel))];
                    textImage[targetIndex + 3] =
                        std::max(textImage[targetIndex + 3], value);
                    textImage[index] = mix(value / 255.f, textImage[index],
                                           textColorParts[subPixel]);
                }
            }
        }

        penX += xAdvance;
        penY += yAdvance;
    }

    //

    hb_buffer_destroy(hbBuffer);

    hb_font_destroy(hbFont);
    FT_Done_Face(ftFace);

    QImage img(textImage, textImageWidth, textImageHeight, textImageWidth * 4,
               QImage::Format::Format_ARGB32_Premultiplied);
    QPixmap pixmap = QPixmap::fromImage(img.copy());
    pixmap.setDevicePixelRatio(scale);
    setPixmap(pixmap);
    setFixedSize(textImageWidth / scale, textImageHeight / scale);
    setScaledContents(true);
    setAlignment(Qt::AlignLeft);
};

bool FontPopupFontPreview::event(QEvent *e) {
    if (e->type() == QEvent::Paint) {
        createInside();
    }
    return QLabel::event(e);
}
