#include "text_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"
#include "render.hpp"

TextElement::TextElement() : Element() {}

TextElementRender::~TextElementRender() {
    if (!hbBuffer)
        return;

    hb_buffer_destroy(hbBuffer);
}

AnimatableRender *TextElement::createClass() { return new TextElementRender(); }

Rect TextElementRender::getRenderBox() {
    return {minX + x, minY + y, maxX - minX, maxY - minY};
}

void TextElementRender::prepare() {
    hbBuffer = hb_buffer_create();
    hb_buffer_add_utf8(hbBuffer, text.get().c_str(), -1, 0, -1);

    hb_buffer_guess_segment_properties(hbBuffer);

    fontInfo = renderThread->getFont(font, 128);

    hb_shape(fontInfo->hb, hbBuffer, nullptr, 0);

    infos = hb_buffer_get_glyph_infos(hbBuffer, &glyphCount);
    positions = hb_buffer_get_glyph_positions(hbBuffer, &glyphCount);

    calculateSize();
}

void TextElementRender::calculateSize() {
    if (fontInfo == nullptr)
        return;

    std::scoped_lock lock{glyphLoadingMutex};
    int curX = 0;
    int curY = 0;

    for (unsigned int i = 0; i < glyphCount; i++) {
        auto codepoint = infos[i].codepoint;
        auto glyph = fontInfo->getGlyph(codepoint);

        auto glyphPos = positions[i];
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
}

bool TextElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();

    int curX = 0;
    int curY = 0;

    for (unsigned int i = 0; i < glyphCount; i++) {
        auto glyphPos = positions[i];
        auto codepoint = infos[i].codepoint;
        auto glyph = fontInfo->getGlyph(infos[i].codepoint);

        int xOffset = (glyphPos.x_offset >> 6);
        int yOffset = (glyphPos.y_offset >> 6);
        int xAdvance = (glyphPos.x_advance >> 6);
        int yAdvance = (glyphPos.y_advance >> 6);

        int drawX = (curX + xOffset + glyph->left) - minX;
        int drawY = (curY + yOffset - glyph->top) - minY;

        bool invalid = false;
        for (unsigned int y = 0; y < glyph->bitmap.rows; y++) {
            for (unsigned int x = 0; x < glyph->bitmap.width; x++) {
                int targetX = drawX + x;
                int targetY = drawY + y;
                if (targetX < 0 || targetY < 0 || targetX >= rect.w ||
                    targetY >= rect.h)
                    continue;
                int index = pixelIndex(targetX, targetY, rect.w);

                switch (glyph->bitmap.pixel_mode) {
                case FT_PIXEL_MODE_GRAY:
                    target[index] = over(
                        target[index],
                        makePixel(
                            255, 255, 255,
                            glyph->bitmap.buffer[y * glyph->bitmap.pitch + x]));
                    break;
                case FT_PIXEL_MODE_BGRA:
                    target[index] =
                        over(target[index], ((uint32_t *)(glyph->bitmap.buffer))
                                                [y * glyph->bitmap.width + x]);
                    break;
                default: {
                    invalid = true;
                    target[index] =
                        over(target[index], makePixel(255, 0, 0, 255));
                }
                }
            }
        }

        if (invalid) {
            qInfo() << "Invalid pixel_mode" << glyph->bitmap.pixel_mode;
        }

        curX += xAdvance;
        curY += yAdvance;
    }

    return true;
}
