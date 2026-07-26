#pragma once
#include "variant.hpp"
#include <QMutex>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftstroke.h>
#include <hb-ft.h>
#include <hb.h>
#include <string>
#include <unordered_map>

extern "C" {
#include <libavutil/frame.h>
}

class FontManager;
class ElementRender;

void convertToAvFrame(AVFrame *frame, uint32_t *source, int w, int h);

class ImageData {
  public:
    ~ImageData();
    int width;
    int height;
    uint32_t *data;
};

class ImageLoader {
  public:
    std::unordered_map<std::string, std::shared_ptr<ImageData>> imageDatas{};
    QMutex imageDatasMutex{};

    std::shared_ptr<ImageData> loadImage(const std::string &path);
};

extern ImageLoader globalImageLoader;

class StrokeInfo {
  public:
    StrokeInfo() : strokeWidth(0), lineJoin(FT_STROKER_LINEJOIN_ROUND) {}
    StrokeInfo(int w, FT_Stroker_LineJoin lj) : strokeWidth(w), lineJoin(lj) {}
    std::string hash() const {
        if (strokeWidth > 0) {
            return std::to_string(strokeWidth) + "\n" +
                   std::to_string(lineJoin);
        } else {
            return "0\n0";
        }
    }
    int strokeWidth;
    FT_Stroker_LineJoin lineJoin;
};

std::string getFontHash(const Font &font, int fontSize, bool antialiased);

class FontInfo {
  public:
    FontInfo(FT_Face face, hb_font_t *hb, int pixelHeight, Font font);
    ~FontInfo();

    FT_BitmapGlyph getGlyph(hb_codepoint_t codepoint);

    std::unordered_map<hb_codepoint_t, FT_BitmapGlyph> glyphs;

    FontManager *fontManager;
    Font font;
    FT_Face face;
    hb_font_t *hb;
    int pixelHeight;
    bool antialiased{true};
    StrokeInfo strokeInfo;
    int framesUnused{0};
};

class FontManager {
  public:
    FontManager();
    ~FontManager();
    FT_Library ftLibrary{nullptr};
    FontInfo *getFont(const Font &font, int fontSize, bool antialiased,
                      const StrokeInfo &strokeInfo);

    void garbageCollect();

  private:
    std::unordered_map<std::string, FontInfo *> loadedFonts;
};

class RenderThread {
  public:
    void init();
    void garbageCollect();
    void close();

    FontManager *fontManager{nullptr};
};

class RenderedElement {
  public:
    RenderedElement(ElementRender *element, int frame, double seconds);
    ~RenderedElement();
    ElementRender *element;

    uint32_t *__restrict__ elementValues;
    uint32_t *__restrict__ finalValues;
    Rect finalRect;
    bool hasError{false};
};
