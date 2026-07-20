#pragma once
#include "element.hpp"
#include "render.hpp"
#include <freetype/freetype.h>
#include <hb-ft.h>
#include <hb.h>

class TextSpan {
  public:
    TextSpan() {}
    Font font;
    int fontSize;
    std::string text;
};

class TextSpans {
  public:
    TextSpans() {}
    QList<TextSpan> spans;
};

class TextElement : public Element {
  public:
    TextElement();
    virtual ~TextElement() {}

    virtual AnimatableRender *createClass();

    Property<Font> font{this, "font", Variant::defaultFont};
    Property<Brush> fill{this, "fill", {}};
    Property<std::string> text{this, "text", "Enter your text"};
};

class TextElementRender : public ElementRender {
  public:
    TextElementRender() : ElementRender() {}
    virtual ~TextElementRender();

    // TODO: TEMP. change to some span thing
    PropertyRender<Font> font{this};
    PropertyRender<Brush> fill{this};
    PropertyRender<std::string> text{this};

    hb_buffer_t *hbBuffer{nullptr};
    FontInfo *fontInfo;

    hb_glyph_info_t *infos;
    hb_glyph_position_t *positions;
    uint32_t glyphCount;

    int minX{INT_MAX};
    int minY{INT_MAX};
    int maxX{INT_MIN};
    int maxY{INT_MIN};

    void calculateSize();

    void prepare() override;
    Rect getRenderBox() override;
    bool render(uint32_t *target) override;
};
