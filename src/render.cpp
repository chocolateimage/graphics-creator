#include "render.hpp"
#include "lua_state.hpp"
#include "math.hpp"
#include "video.hpp"
#include <freetype/ftglyph.h>
#include <iostream>
#include <mutex>
#include <unistd.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

std::mutex glyphLoadingMutex;
std::unordered_map<std::string, FT_BitmapGlyph> glyphMap;

FontInfo::FontInfo(FT_Face face, hb_font_t *hb, int pixelHeight)
    : face(face), hb(hb), pixelHeight(pixelHeight) {};

FontInfo::~FontInfo() {
    // TODO: glyphMap but in a better way. right now deleting a FontInfo causes
    // all glyphMaps of a font to get deleted
    glyphLoadingMutex.lock();
    for (auto it = glyphMap.begin(); it != glyphMap.end();) {
        if (it->first.rfind(std::string(face->family_name) + ";", 0) == 0) {
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
    std::string hash =
        std::string(face->family_name) + ";" + std::to_string(codepoint);

    std::scoped_lock lock{glyphLoadingMutex};

    auto it = glyphMap.find(hash);
    if (it != glyphMap.end()) {
        return it->second;
    }

    FT_Glyph _glyph;
    FT_Load_Glyph(face, codepoint,
                  FT_LOAD_RENDER | FT_LOAD_NO_BITMAP |
                      FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
    FT_Get_Glyph(face->glyph, &_glyph);

    FT_BitmapGlyph glyph = (FT_BitmapGlyph)_glyph;
    glyphMap.emplace(hash, glyph);
    return glyph;
}

void RenderThread::init() { FT_Init_FreeType(&ftLibrary); }

int TextClass__gc(lua_State *L) {
    Text **userdata = (Text **)luaL_checkudata(L, 1, "TextClass");

    Text *text = *userdata;
    delete text;
    *userdata = nullptr;

    return 0;
}

int TextClass_create(lua_State *L) {
    int arguments = lua_gettop(L);
    if (arguments != 2) {
        luaL_error(L, "createText(): invalid argument count");
        return 0;
    }
    auto text = lua_tostring(L, 1);
    if (text == nullptr) {
        luaL_error(L, "createText(): invalid text");
        return 0;
    }
    RenderThread *renderThread =
        (RenderThread *)lua_touserdata(L, lua_upvalueindex(1));
    Text *textObject = new Text(renderThread, text);
    Text **newUserData = (Text **)lua_newuserdata(L, sizeof(Text *));
    *newUserData = textObject;
    luaL_getmetatable(L, "TextClass");
    lua_setmetatable(L, -2);
    return 1;
}

int TextClass_getPixel(lua_State *L) {
    int arguments = lua_gettop(L);
    if (arguments != 4) {
        luaL_error(L, "getPixel(): invalid argument count (textInstance, "
                      "fontSize, x, y)");
        return 0;
    }

    Text *text = *((Text **)luaL_checkudata(L, 1, "TextClass"));
    float fontSize = lua_tonumber(L, 2);
    float x = lua_tonumber(L, 3);
    float y = lua_tonumber(L, 4);

    lua_pushnumber(L, text->getSmoothPixel(fontSize, x, y));
    return 1;
}

int TextClass_getTextInfo(lua_State *L) {
    int arguments = lua_gettop(L);
    if (arguments != 2) {
        luaL_error(
            L,
            "getTextInfo(): invalid argument count (textInstance, fontSize)");
        return 0;
    }

    Text *text = *((Text **)luaL_checkudata(L, 1, "TextClass"));

    return text->luaGetInfo(L, lua_tonumber(L, 2));
}

Text::Text(RenderThread *renderThread, const char *text)
    : renderThread(renderThread) {
    fontInfo = renderThread->getFont(
        "/usr/share/fonts/noto/NotoSans-Bold.ttf"); // TODO: dynamic font (with
                                                    // options)

    hbBuffer = hb_buffer_create();
    hb_buffer_add_utf8(hbBuffer, text, -1, 0, -1);

    hb_buffer_guess_segment_properties(hbBuffer);

    hb_shape(fontInfo->hb, hbBuffer, nullptr, 0);

    glyphInfo = hb_buffer_get_glyph_infos(hbBuffer, &glyphCount);
    glyphPositions = hb_buffer_get_glyph_positions(hbBuffer, &glyphCount);

    calculateSize();
    draw();
}

void Text::calculateSize() {
    int curX = 0;
    int curY = 0;

    for (unsigned int i = 0; i < glyphCount; i++) {
        auto codepoint = glyphInfo[i].codepoint;
        auto glyph = fontInfo->getGlyph(codepoint);

        auto glyphPos = glyphPositions[i];
        int xOffset = glyphPos.x_offset >> 6;
        int yOffset = glyphPos.y_offset >> 6;
        int xAdvance = glyphPos.x_advance >> 6;
        int yAdvance = glyphPos.y_advance >> 6;

        int drawX = (curX + xOffset + glyph->left);
        int drawY = (curY + yOffset - glyph->top);

        minX = std::min(minX, drawX);
        minY = std::min(minY, drawY);
        maxX = std::max(maxX, (int)glyph->bitmap.width + drawX);
        maxY = std::max(maxY, (int)glyph->bitmap.rows + drawY);

        curX += xAdvance;
        curY += yAdvance;
    }

    width = (maxX - minX);
    height = (maxY - minY);
}

void Text::draw() {
    int curX = 0;
    int curY = 0;

    image = new uint8_t[width * height];
    memset(image, 0, width * height);

    for (unsigned int i = 0; i < glyphCount; i++) {
        auto glyphPos = glyphPositions[i];
        auto codepoint = glyphInfo[i].codepoint;
        int xOffset = (glyphPos.x_offset >> 6);
        int yOffset = (glyphPos.y_offset >> 6);
        int xAdvance = (glyphPos.x_advance >> 6);
        int yAdvance = (glyphPos.y_advance >> 6);
        auto glyph = fontInfo->getGlyph(glyphInfo[i].codepoint);

        int drawX = (curX + xOffset + glyph->left) - minX;
        int drawY = (curY + yOffset - glyph->top) - minY;

        for (unsigned int y = 0; y < glyph->bitmap.rows; y++) {
            for (unsigned int x = 0; x < glyph->bitmap.width; x++) {
                int targetX = drawX + x;
                int targetY = drawY + y;
                image[targetY * width + targetX] =
                    std::max(image[targetY * width + targetX],
                             glyph->bitmap.buffer[y * glyph->bitmap.pitch + x]);
            }
        }

        curX += xAdvance;
        curY += yAdvance;
    }
}

inline uint8_t Text::getPixel(int x, int y) {
    if (x < 0 || y < 0 || x >= width || y >= height)
        return 0;

    return image[y * width + x];
}

float Text::getSmoothPixel(float fontSize, float x, float y) {
    float adjust = fontSize / fontInfo->pixelHeight;
    float targetX = x / adjust;
    float targetY = y / adjust;
    int leftX = std::floor(targetX);
    int rightX = leftX + 1;
    int topY = std::floor(targetY);
    int bottomY = topY + 1;
    float topLeft = (float)getPixel(leftX, topY) / 255 - 0.5;
    float topRight = (float)getPixel(rightX, topY) / 255 - 0.5;
    float bottomLeft = (float)getPixel(leftX, bottomY) / 255 - 0.5;
    float bottomRight = (float)getPixel(rightX, bottomY) / 255 - 0.5;

    float xPercent = targetX - leftX;
    float yPercent = targetY - topY;
    float topValue = mix(xPercent, topLeft, topRight);
    float bottomValue = mix(xPercent, bottomLeft, bottomRight);

    return mix(yPercent, topValue, bottomValue);
}

int Text::luaGetInfo(lua_State *L, float fontSize) {
    float adjust = fontSize / fontInfo->pixelHeight;

    lua_pushnumber(L, (float)minX * adjust);
    lua_pushnumber(L, (float)minY * adjust);
    lua_pushnumber(L, (float)maxX * adjust);
    lua_pushnumber(L, (float)maxY * adjust);
    lua_pushnumber(L, (float)width * adjust);
    lua_pushnumber(L, (float)height * adjust);
    return 6;
}

Text::~Text() {
    hb_buffer_destroy(hbBuffer);
    delete[] image;
}

FontInfo *RenderThread::getFont(const std::string &font) {
    auto it = loadedFonts.find(font);
    if (it != loadedFonts.end()) {
        return it->second;
    }

    int pixelHeight = 128;

    FT_Face ftFace;
    FT_New_Face(ftLibrary, font.c_str(), 0, &ftFace);
    FT_Set_Pixel_Sizes(ftFace, 0, pixelHeight);

    hb_font_t *hbFont = hb_ft_font_create(ftFace, nullptr);
    FontInfo *fontInfo = new FontInfo(ftFace, hbFont, pixelHeight);
    loadedFonts.emplace(font, fontInfo);
    return fontInfo;
}

bool RenderThread::loadLua(const std::string &code) {
    if (L) {
        lua_close(L);
        L = nullptr;
    }

    L = createLuaState();

    // createText()
    luaL_newmetatable(L, "TextClass");
    lua_pushcfunction(L, TextClass__gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, TextClass_create, 1);
    lua_setglobal(L, "createText");

    lua_pushcfunction(L, TextClass_getPixel);
    lua_setglobal(L, "getPixel");

    lua_pushcfunction(L, TextClass_getTextInfo);
    lua_setglobal(L, "getTextInfo");

    // load
    if (luaL_dostring(L, code.c_str()) != LUA_OK) {
        auto err = lua_tostring(L, -1);
        lastError = err;
        lua_pop(L, 1);
        return false;
    }
    return true;
}

void RenderThread::updateOptions(
    const std::map<std::string, Variant> &variants) {
    options.clear();

    for (auto variant : variants) {
        options.emplace(variant.first, variant.second);
    }
}

void RenderThread::close() {
    destroyPlanes();
    if (L) {
        lua_close(L);
        L = nullptr;
    }
    for (auto loadedFont : loadedFonts) {
        delete loadedFont.second;
    }
    loadedFonts.clear();
    FT_Done_FreeType(ftLibrary);
}

void RenderThread::destroyPlanes() {
    if (values != nullptr) {
        delete[] values;
        values = nullptr;
        lastWidth = 0;
        lastHeight = 0;
    }
}

void RenderThread::createPlanes(int width, int height) {
    if (width == lastWidth && height == lastHeight)
        return;
    destroyPlanes();
    lastWidth = width;
    lastHeight = height;
    values = new uint32_t[width * height];
}

bool RenderThread::drawImage(Video *video, AVFrame *frame, int startX,
                             int startY, int renderWidth, int renderHeight) {
    if (!L) {
        std::cerr << "impossible..." << std::endl;
        lastError = "No Lua state found";
        return false;
    }

    int width = frame->width;
    int height = frame->height;
    int pixelFormat = frame->format;
    int frameIndex = frame->pts;

    float seconds = (float)frameIndex / video->frameRate;

    lua_pushnumber(L, frameIndex);
    lua_setglobal(L, "frameIndex");

    lua_pushnumber(L, video->frameRate);
    lua_setglobal(L, "frameRate");

    lua_pushnumber(L, seconds);
    lua_setglobal(L, "seconds");

    lua_pushnumber(L, video->duration);
    lua_setglobal(L, "duration");

    lua_pushnumber(L, width);
    lua_setglobal(L, "width");

    lua_pushnumber(L, height);
    lua_setglobal(L, "height");

    lua_pushnumber(L, startX);
    lua_setglobal(L, "fromX");

    lua_pushnumber(L, startX + renderWidth - 1);
    lua_setglobal(L, "toX");

    lua_pushnumber(L, startY);
    lua_setglobal(L, "fromY");

    lua_pushnumber(L, startY + renderHeight - 1);
    lua_setglobal(L, "toY");

    for (const auto &option : options) {
        option.second.pushLua(L);
        lua_setglobal(L, option.first.c_str());
    }

    lua_getglobal(L, "draw");
    createPlanes(width, height);
    lua_pushlightuserdata(L, values);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
        auto err = lua_tostring(L, -1);
        lastError = err;
        lua_pop(L, 1);
        return false;
    }

    for (int y = startY; y < startY + renderHeight; y++) {
        for (int x = startX; x < startX + renderWidth; x++) {
            uint32_t value = values[y * width + x];
            if (pixelFormat == AV_PIX_FMT_BGRA) {
                ((uint32_t *)(frame->data[0]))[y * (frame->linesize[0] / 4) +
                                               x] = value;
                continue;
            }

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
            }
        }
    }
    return true;
}
