#pragma once
#include "lua.hpp"
#include "variant.hpp"
#include "video.hpp"
#include <freetype/ftglyph.h>
#include <ft2build.h>
#include <memory>
#include <string>
#include FT_FREETYPE_H
#include <hb-ft.h>
#include <hb.h>
#include <map>
#include <unordered_map>
#include <vector>

extern "C" {
#include <libavutil/frame.h>
}

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
    bool loadLua(const std::string &code);
    void updateOptions(const std::map<std::string, Variant> &variants);
    bool drawImage(std::shared_ptr<Video> video, AVFrame *frame, int startX,
                   int startY, int renderWidth, int renderHeight);
    void close();
    FontInfo *getFont(const Font &font);
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
    int framesSinceLastGc{0};
};

class Text {
  public:
    Text(RenderThread *renderThread, const char *text, const Font &font,
         bool isManual);
    uint8_t getPixel(int x, int y, int index);
    float getSmoothPixel(float fontSize, float x, float y, int index);
    int luaGetInfo(lua_State *L, float fontSize);
    int luaGetIndexInfo(lua_State *L, float fontSize, int index);
    int luaGetAllCharsInfo(lua_State *L, float fontSize);
    ~Text();
    std::vector<uint32_t> glyphCounts;
    uint32_t totalGlyphs{0};

  private:
    void calculateSize();
    void draw();
    int minX{INT_MAX};
    int minY{INT_MAX};
    int maxX{INT_MIN};
    int maxY{INT_MIN};
    int width;
    int height;
    bool isManual{false};
    std::vector<std::pair<int, int>> drawPoints;
    std::vector<std::pair<int, int>> offsetPoints;
    std::vector<std::tuple<uint8_t *, int, int>> bitmaps;
    RenderThread *renderThread;
    FontInfo *fontInfo{nullptr};
    std::vector<hb_buffer_t *> hbBuffers;
    std::vector<hb_glyph_info_t *> glyphsInfo;
    std::vector<hb_glyph_position_t *> glyphsPositions;
    uint8_t *image{nullptr};
};
