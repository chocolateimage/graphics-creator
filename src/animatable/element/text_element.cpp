#include "text_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"
#include "render.hpp"

TextElement::TextElement() : Element() {}

TextElementRender::~TextElementRender() {
    for (auto buffer : hbBuffers) {
        hb_buffer_destroy(buffer);
    }
}

AnimatableRender *TextElement::createClass() { return new TextElementRender(); }

Rect TextElementRender::getRenderBox() {
    return {minX + x, minY + y, maxX - minX, maxY - minY};
}

void TextElementRender::prepare() {
    std::string a = "Hello world";
    int c = 0;
    for (auto b : a) {
        TextSpan span1;
        span1.text = b;
        span1.fill = {};
        span1.font = test1;
        span1.fontSize = test1Size + std::sin(c / 2.) * 50;
        text.value.spans.append(std::move(span1));
        c++;
    }

    for (auto span : text.get().spans) {
        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, span.text.c_str(), -1, 0, -1);

        hb_buffer_guess_segment_properties(hbBuffer);

        FontInfo *fontInfo =
            renderThread->getFont(span.font, std::max(1, span.fontSize));
        fontInfos.push_back(fontInfo);

        hb_shape(fontInfo->hb, hbBuffer, nullptr, 0);

        uint32_t glyphCount;
        infos.push_back(hb_buffer_get_glyph_infos(hbBuffer, &glyphCount));
        positions.push_back(
            hb_buffer_get_glyph_positions(hbBuffer, &glyphCount));
        glyphCounts.push_back(glyphCount);

        hbBuffers.push_back(hbBuffer);
        spanCount++;
    }

    calculateSize();
}

void TextElementRender::calculateSize() {
    std::scoped_lock lock{glyphLoadingMutex};
    int curX = 0;
    int curY = 0;

    for (int si = 0; si < spanCount; si++) {
        for (unsigned int i = 0; i < glyphCounts[si]; i++) {
            auto codepoint = infos[si][i].codepoint;
            auto glyph = fontInfos[si]->getGlyph(codepoint);

            auto glyphPos = positions[si][i];
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
}

bool TextElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();

    int curX = 0;
    int curY = 0;

    for (int si = 0; si < spanCount; si++) {
        for (unsigned int i = 0; i < glyphCounts[si]; i++) {
            auto glyphPos = positions[si][i];
            auto codepoint = infos[si][i].codepoint;
            auto glyph = fontInfos[si]->getGlyph(infos[si][i].codepoint);

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
                        target[index] =
                            over(target[index],
                                 makePixel(
                                     255, 255, 255,
                                     glyph->bitmap
                                         .buffer[y * glyph->bitmap.pitch + x]));
                        break;
                    case FT_PIXEL_MODE_BGRA:
                        target[index] = over(
                            target[index], ((uint32_t *)(glyph->bitmap.buffer))
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
    }

    return true;
}
