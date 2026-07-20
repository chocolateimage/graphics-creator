#pragma once
#include "variant.hpp"
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <hb-ft.h>
#include <hb.h>
#include <mutex>
#include <string>
#include <unordered_map>

static std::mutex glyphLoadingMutex;

class FontInfo {
  public:
    FontInfo(FT_Face face, hb_font_t *hb, int pixelHeight, Font font);
    ~FontInfo();

    FT_BitmapGlyph getGlyph(hb_codepoint_t codepoint);

    Font font;
    FT_Face face;
    hb_font_t *hb;
    int pixelHeight;
    int framesUnused{0};
};

class RenderThread {
  public:
    void init();
    void close();

    FT_Library ftLibrary{nullptr};
    FontInfo *getFont(const Font &font, int fontSize);

  private:
    std::unordered_map<std::string, FontInfo *> loadedFonts;
};
