#pragma once
#include "element.hpp"
#include "render.hpp"
#include <freetype/freetype.h>
#include <hb-ft.h>
#include <hb.h>

class TextLayoutItem {
  public:
    // Starting from 0
    int line;
    // The points are aligned from the bottom
    QPoint startPoint;
    QPoint endPoint;
    QPoint selectionEndPoint;

    int height;
};

class TextLayout {
  public:
    QList<int> lineHeights{};
    QList<TextLayoutItem> items{};
};

class TextElement : public Element {
  public:
    TextElement();
    virtual ~TextElement();

    TextSpan createDefaultTextSpan();

    FontManager *fontManager;
    AnimatableRender *createClass() override;
    QRect getBoundingBox(const FrameInfo &frameInfo) override;

    TextLayout layTheTextOut(const FrameInfo &frameInfo);

    Property<TextSpans> text{this, "text", {}};
    Property<Brush> testFill{this, "testFill", {}};
    Property<Font> test1{this, "test1", Variant::defaultFont};
    Property<int> test1Size{this, "test1Size", 128};
    Property<Font> test2{this, "test2", Variant::defaultFont};
    Property<int> test2Size{this, "test2Size", 64};
};

class TextElementRender : public ElementRender {
  public:
    TextElementRender() : ElementRender() {}
    virtual ~TextElementRender();

    PropertyRender<TextSpans> text{this};
    PropertyRender<Brush> testFill{this};
    PropertyRender<Font> test1{this};
    PropertyRender<int> test1Size{this};
    PropertyRender<Font> test2{this};
    PropertyRender<int> test2Size{this};

    int spanCount{0};
    std::vector<hb_buffer_t *> hbBuffers;
    std::vector<FontInfo *> fontInfos;

    std::vector<hb_glyph_info_t *> infos;
    std::vector<hb_glyph_position_t *> positions;
    std::vector<uint32_t> glyphCounts;

    int minX{INT_MAX};
    int minY{INT_MAX};
    int maxX{INT_MIN};
    int maxY{INT_MIN};
    int moveDown{0};

    void calculateSize();

    void prepare() override;
    Rect getRenderBox() override;
    bool render(uint32_t *target) override;
};
