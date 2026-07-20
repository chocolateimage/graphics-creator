#include "render.hpp"
#include <QDebug>
#include <freetype/ftoutln.h>

FontInfo::FontInfo(FT_Face face, hb_font_t *hb, int pixelHeight, Font font)
    : font(font), face(face), hb(hb), pixelHeight(pixelHeight) {};

FontInfo::~FontInfo() {
    static int deleteCount = 0;
    for (auto &glyph : glyphs) {
        FT_Done_Glyph((FT_Glyph)(glyph.second));
    }
    hb_font_destroy(hb);
    FT_Done_Face(face);
}

FT_BitmapGlyph FontInfo::getGlyph(hb_codepoint_t codepoint) {
    auto it = glyphs.find(codepoint);
    if (it != glyphs.end()) {
        return it->second;
    }

    FT_Glyph _glyph;
    FT_Load_Glyph(face, codepoint, FT_LOAD_COLOR);
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    FT_Get_Glyph(face->glyph, &_glyph);

    FT_BitmapGlyph glyph = (FT_BitmapGlyph)_glyph;
    glyphs.emplace(codepoint, glyph);
    return glyph;
}

void RenderThread::init() { FT_Init_FreeType(&ftLibrary); }

FontInfo *RenderThread::getFont(const Font &font, int fontSize) {
    auto it = loadedFonts.find(getFontHash(font, fontSize));
    if (it != loadedFonts.end()) {
        FontInfo *fontInfo = it->second;
        fontInfo->framesUnused = 0;
        return fontInfo;
    }

    int pixelHeight = fontSize;

    FT_Face ftFace;
    FT_New_Face(ftLibrary, font.path.c_str(), font.index, &ftFace);
    FT_Set_Pixel_Sizes(ftFace, 0, pixelHeight);

    hb_font_t *hbFont = hb_ft_font_create(ftFace, nullptr);
    FontInfo *fontInfo = new FontInfo(ftFace, hbFont, pixelHeight, font);
    loadedFonts.emplace(getFontHash(font, fontSize), fontInfo);
    return fontInfo;
}

void RenderThread::garbageCollect() {
    std::vector<std::string> toRemove;
    for (auto &fontInfo : loadedFonts) {
        if (++fontInfo.second->framesUnused > 10) {
            delete fontInfo.second;
            toRemove.push_back(fontInfo.first);
        }
    }
    for (const auto &key : toRemove) {
        loadedFonts.erase(key);
    }
}

void RenderThread::close() {
    for (auto loadedFont : loadedFonts) {
        delete loadedFont.second;
    }
    loadedFonts.clear();
    FT_Done_FreeType(ftLibrary);
}
