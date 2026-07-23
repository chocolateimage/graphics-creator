#include "render.hpp"
#include "math.hpp"
#include <QDebug>
#include <freetype/ftoutln.h>

std::string getFontHash(const Font &font, int fontSize, bool antialiased,
                        const StrokeInfo &strokeInfo) {
    return font.path + "\n" + std::to_string(fontSize) + "\n" +
           (antialiased ? "1" : "0") + "\n" + std::to_string(font.index) +
           "\n" + strokeInfo.hash();
}

void convertToAvFrame(AVFrame *frame, uint32_t *source, int w, int h) {
    AVPixelFormat pixelFormat = (AVPixelFormat)frame->format;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint32_t value = source[y * w + x];

            uint8_t blue = value & 0xff;
            uint8_t green = value >> 8 & 0xff;
            uint8_t red = value >> 16 & 0xff;
            uint8_t alpha = value >> 24 & 0xff;

            float Y = 0.299 * red + 0.587 * green + 0.114 * blue;

            switch (pixelFormat) {
            case AV_PIX_FMT_YUV420P: {
                if (alpha < 255) {
                    uint8_t transparencyPixel =
                        (((x / 10) % 2 == ((y / 10) % 2 == 0))) ? 255 : 200;
                    red = mix(alpha / 255.f, transparencyPixel, red);
                    green = mix(alpha / 255.f, transparencyPixel, green);
                    blue = mix(alpha / 255.f, transparencyPixel, blue);
                    Y = 0.299 * red + 0.587 * green + 0.114 * blue;
                }
                frame->data[0][y * frame->linesize[0] + x] = Y;

                if (x % 2 == 0 && y % 2 == 0) {
                    float Cb =
                        -0.169 * red - 0.331 * green + 0.500 * blue + 128;
                    float Cr = 0.500 * red - 0.419 * green - 0.081 * blue + 128;
                    frame->data[1][(y >> 1) * frame->linesize[1] + (x >> 1)] =
                        Cb;
                    frame->data[2][(y >> 1) * frame->linesize[2] + (x >> 1)] =
                        Cr;
                }
                break;
            }
            case AV_PIX_FMT_YUVA420P: {
                frame->data[0][y * frame->linesize[0] + x] = Y;
                frame->data[3][y * frame->linesize[3] + x] = alpha;

                if (x % 2 == 0 && y % 2 == 0) {
                    float Cb =
                        -0.169 * red - 0.331 * green + 0.500 * blue + 128;
                    float Cr = 0.500 * red - 0.419 * green - 0.081 * blue + 128;
                    frame->data[1][(y >> 1) * frame->linesize[1] + (x >> 1)] =
                        Cb;
                    frame->data[2][(y >> 1) * frame->linesize[2] + (x >> 1)] =
                        Cr;
                }
                break;
            }
            case AV_PIX_FMT_YUVA444P10LE: {
                float Cb = -0.169 * red - 0.331 * green + 0.500 * blue + 128;
                float Cr = 0.500 * red - 0.419 * green - 0.081 * blue + 128;
                ((uint16_t *)(frame->data[0] + y * frame->linesize[0]))[x] =
                    Y * 4;
                ((uint16_t *)(frame->data[1] + y * frame->linesize[1]))[x] =
                    Cb * 4;
                ((uint16_t *)(frame->data[2] + y * frame->linesize[2]))[x] =
                    Cr * 4;
                ((uint16_t *)(frame->data[3] + y * frame->linesize[3]))[x] =
                    alpha * 4;
                break;
            }
            case AV_PIX_FMT_GBRAP: {
                frame->data[0][y * frame->linesize[0] + x] = green;
                frame->data[1][y * frame->linesize[1] + x] = blue;
                frame->data[2][y * frame->linesize[2] + x] = red;
                frame->data[3][y * frame->linesize[3] + x] = alpha;
                break;
            }
            case AV_PIX_FMT_ARGB: {
                frame->data[0][y * frame->linesize[0] + x * 4] = alpha;
                frame->data[0][y * frame->linesize[0] + x * 4 + 1] = red;
                frame->data[0][y * frame->linesize[0] + x * 4 + 2] = green;
                frame->data[0][y * frame->linesize[0] + x * 4 + 3] = blue;
                break;
            }
            default: {
                break;
            }
            }
        }
    }
}

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
    FT_Load_Glyph(face, codepoint, FT_LOAD_COLOR | FT_LOAD_NO_HINTING);

    FT_Render_Mode renderMode =
        antialiased ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;

    if (strokeInfo.strokeWidth > 0) {
        FT_Stroker stroker;
        FT_Stroker_New(fontManager->ftLibrary, &stroker);
        FT_Stroker_Set(stroker, pixelHeight * strokeInfo.strokeWidth,
                       FT_STROKER_LINECAP_SQUARE, strokeInfo.lineJoin, 0);
        FT_Get_Glyph(face->glyph, &_glyph);
        FT_Glyph_StrokeBorder(&_glyph, stroker, false, true);
        FT_Glyph_To_Bitmap(&_glyph, renderMode, 0, true);
        FT_Stroker_Done(stroker);
    } else {
        FT_Render_Glyph(face->glyph, renderMode);
        FT_Get_Glyph(face->glyph, &_glyph);
    }

    FT_BitmapGlyph glyph = (FT_BitmapGlyph)_glyph;
    glyphs.emplace(codepoint, glyph);
    return glyph;
}

FontManager::FontManager() { FT_Init_FreeType(&ftLibrary); }

FontInfo *FontManager::getFont(const Font &font, int fontSize, bool antialiased,
                               const StrokeInfo &strokeInfo) {
    std::string hash = getFontHash(font, fontSize, antialiased, strokeInfo);
    auto it = loadedFonts.find(hash);
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
    fontInfo->antialiased = antialiased;
    fontInfo->strokeInfo = strokeInfo;
    fontInfo->fontManager = this;
    loadedFonts.emplace(hash, fontInfo);
    return fontInfo;
}

void FontManager::garbageCollect() {

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

FontManager::~FontManager() {
    for (auto loadedFont : loadedFonts) {
        delete loadedFont.second;
    }
    loadedFonts.clear();
    FT_Done_FreeType(ftLibrary);
}

void RenderThread::init() { fontManager = new FontManager(); }

void RenderThread::garbageCollect() { fontManager->garbageCollect(); }

void RenderThread::close() {
    delete fontManager;
    fontManager = nullptr;
}
