#pragma once
#include "lua.hpp"
#include "variant.hpp"
#include "video.hpp"
#include <freetype/ftglyph.h>
#include <ft2build.h>
#include <string>
#include FT_FREETYPE_H
#include <hb-ft.h>
#include <hb.h>
#include <map>
#include <unordered_map>

extern "C" {
#include <libavutil/frame.h>
}

class FontInfo {
  public:
    FontInfo(FT_Face face, hb_font_t *hb, int pixelHeight);
    ~FontInfo();

    FT_BitmapGlyph getGlyph(hb_codepoint_t codepoint);

    FT_Face face;
    hb_font_t *hb;
    int pixelHeight;
};

class RenderThread {
  public:
    void init();
    bool loadLua(const std::string &code);
    void updateOptions(const std::map<std::string, Variant> &variants);
    bool drawImage(Video *video, AVFrame *frame, int startX, int startY,
                   int renderWidth, int renderHeight);
    void close();
    FontInfo *getFont(const std::string &font);
    std::string lastError;
    FT_Library ftLibrary{nullptr};

  private:
    std::unordered_map<std::string, FontInfo *> loadedFonts;
    std::unordered_map<std::string, Variant> options;
    lua_State *L{nullptr};
    void createPlanes(int width, int height);
    void destroyPlanes();
    uint32_t *values{nullptr};
    int lastWidth{0};
    int lastHeight{0};
};

class Text {
  public:
    Text(RenderThread *renderThread, const char *text);
    uint8_t getPixel(int x, int y);
    float getSmoothPixel(float fontSize, float x, float y);
    int luaGetInfo(lua_State *L, float fontSize);
    ~Text();

  private:
    void calculateSize();
    void draw();
    int minX{INT_MAX};
    int minY{INT_MAX};
    int maxX{INT_MIN};
    int maxY{INT_MIN};
    int width;
    int height;
    RenderThread *renderThread;
    FontInfo *fontInfo;
    hb_buffer_t *hbBuffer;
    uint32_t glyphCount;
    hb_glyph_info_t *glyphInfo;
    hb_glyph_position_t *glyphPositions;
    uint8_t *image{nullptr};
};
