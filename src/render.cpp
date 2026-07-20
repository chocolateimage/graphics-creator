#include "render.hpp"
#include <QDebug>
#include <freetype/ftoutln.h>
#include <mutex>

std::unordered_map<std::string, FT_BitmapGlyph> glyphMap;

FontInfo::FontInfo(FT_Face face, hb_font_t *hb, int pixelHeight, Font font)
    : font(font), face(face), hb(hb), pixelHeight(pixelHeight) {};

FontInfo::~FontInfo() {
    // TODO: glyphMap but in a better way. right now deleting a FontInfo causes
    // all glyphMaps of a font to get deleted
    glyphLoadingMutex.lock();
    for (auto it = glyphMap.begin(); it != glyphMap.end();) {
        if (it->first.rfind(getFontHash(font) + ";", 0) == 0) {
            FT_Done_Glyph((FT_Glyph)it->second);
            it = glyphMap.erase(it);
        } else {
            it++;
        }
    }
    glyphLoadingMutex.unlock();
    hb_font_destroy(hb);
    FT_Done_Face(face);
}

FT_BitmapGlyph FontInfo::getGlyph(hb_codepoint_t codepoint) {
    // TODO: use better hash. this is terrible.
    std::string hash = getFontHash(font) + "\n" + std::to_string(codepoint);

    auto it = glyphMap.find(hash);
    if (it != glyphMap.end()) {
        return it->second;
    }

    FT_Glyph _glyph;
    FT_Load_Glyph(face, codepoint, FT_LOAD_COLOR);
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    FT_Get_Glyph(face->glyph, &_glyph);

    FT_BitmapGlyph glyph = (FT_BitmapGlyph)_glyph;
    glyphMap.emplace(hash, glyph);
    return glyph;
}

void RenderThread::init() { FT_Init_FreeType(&ftLibrary); }

FontInfo *RenderThread::getFont(const Font &font, int fontSize) {
    auto it = loadedFonts.find(getFontHash(font));
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
    loadedFonts.emplace(getFontHash(font), fontInfo);
    return fontInfo;
}

void RenderThread::close() {
    for (auto loadedFont : loadedFonts) {
        delete loadedFont.second;
    }
    loadedFonts.clear();
    FT_Done_FreeType(ftLibrary);
}
