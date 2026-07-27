#pragma once
#include "element.hpp"
#include "render.hpp"
#include <freetype/freetype.h>
#include <hb-ft.h>
#include <hb.h>

class TextElement;

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

    int width{0};
    int height{0};
};

class TextAnimatorSelectorRender : public AnimatableRender {
  public:
    PropertyRender<double> start{this};
    PropertyRender<double> end{this};
    PropertyRender<double> offset{this};

    double percent(int character, int totalCharacters, int word, int totalWords,
                   int line, int totalLine);
};

class TextAnimatorSelector : public Animatable {
  public:
    AnimatableRender *createClass() override;
    Property<double> start{this, "start", 0};
    Property<double> end{this, "end", 100};
    Property<double> offset{this, "offset", 100};
};

class TextAnimatorRender : public AnimatableRender {
  public:
    ~TextAnimatorRender();
    QList<TextAnimatorSelectorRender *> selectors;

    PropertyRender<double> x{this};
    PropertyRender<double> y{this};
};

class TextAnimator : public Animatable, public ICollapsible {
  public:
    ~TextAnimator();
    AnimatableRender *createClass() override;
    AnimatableRender *toRender(const FrameInfo &frameInfo) override;
    TextElement *textElement;

    QString displayName() override;

    bool collapsed{false};
    bool isCollapsed() override;
    void setCollapsed(bool newValue) override;

    QList<TextAnimatorSelector *> selectors;

    Property<double> x{this, "x", 0};
    Property<double> y{this, "y", 0};

    void _propertyUpdated(PropertyBase *property) override;
    void _propertyIsAnimatingUpdated(PropertyBase *property) override;
};

TextLayout layoutText(FontManager *fontManager, const TextSpans &spans,
                      int width,
                      const QList<TextAnimatorRender *> &textAnimators);

class TextElement : public Element {
  public:
    TextElement();
    virtual ~TextElement();

    TextSpan createDefaultTextSpan();

    FontManager *fontManager;
    AnimatableRender *createClass() override;
    QRect getBoundingBox(const FrameInfo &frameInfo) override;
    AnimatableRender *toRender(const FrameInfo &frameInfo) override;
    QList<TextAnimatorRender *> toRenderAnimators(const FrameInfo &frameInfo);

    TextLayout layTheTextOut(const FrameInfo &frameInfo);

    Property<TextSpans> text{this, "text", {}};

    QList<TextAnimator *> textAnimators;

    QString const typeName() override { return "text"; }
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
    std::vector<FontInfo *> strokeFontInfos;

    std::vector<hb_glyph_info_t *> infos;
    std::vector<hb_glyph_position_t *> positions;
    std::vector<uint32_t> glyphCounts;

    TextLayout layout;
    QList<TextAnimatorRender *> textAnimators;

    int minX{INT_MAX};
    int minY{INT_MAX};
    int maxX{INT_MIN};
    int maxY{INT_MIN};

    void calculateSize();

    void prepare() override;
    Rect getRenderBox() override;
    bool render(uint32_t *target) override;
    void renderGlyph(uint32_t *target, FT_BitmapGlyph glyph, int x, int y,
                     Rect &rect, Brush &brush);
};
