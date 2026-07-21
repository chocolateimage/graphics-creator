#include "text_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"
#include "render.hpp"

TextElement::TextElement() : Element() {
    TextSpans &spans = ((Keyframe<TextSpans> *)(text.keyframes[0]))->value;
    std::string defaultText = "Hello world";
    for (auto character : defaultText) {
        TextSpan defaultSpan = createDefaultTextSpan();
        defaultSpan.text = character;
        spans.spans.append(std::move(defaultSpan));
    }
}

TextSpan TextElement::createDefaultTextSpan() {
    TextSpan defaultSpan;
    defaultSpan.text = "";
    defaultSpan.fill = {};
    defaultSpan.font = Variant::defaultFont;
    defaultSpan.fontSize = 128;
    return defaultSpan;
}

QRect TextElement::getBoundingBox(const FrameInfo &frameInfo) {
    if (w.get(frameInfo) != 1 || h.get(frameInfo) != 1) {
        return Element::getBoundingBox(frameInfo);
    }

    TextSpans spans = text.get(frameInfo);
    int tempW = 0; // TEMP
    int maxHeight = 10;
    for (auto span : spans.spans) {
        tempW += span.getLength() * span.fontSize * 0.5;
        maxHeight = std::max(maxHeight, span.fontSize);
    }
    return {x.get(frameInfo), y.get(frameInfo) - maxHeight, tempW, maxHeight};
}

AnimatableRender *TextElement::createClass() { return new TextElementRender(); }

Rect TextElementRender::getRenderBox() {
    int yOffset = 0;
    if (w.get() != 1 || h.get() != 1) {
        yOffset += moveDown;
    }
    return {minX + x, minY + y + yOffset, std::max(1, maxX - minX),
            std::max(1, maxY - minY)};
}

void TextElementRender::prepare() {
    for (auto span : text.get().spans) {
        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, qPrintable(span.text), -1, 0, -1);

        hb_buffer_guess_segment_properties(hbBuffer);

        FontInfo *fontInfo = renderThread->fontManager->getFont(
            span.font, std::max(1, span.fontSize));
        fontInfos.push_back(fontInfo);

        hb_shape(fontInfo->hb, hbBuffer, nullptr, 0);

        uint32_t glyphCount;
        infos.push_back(hb_buffer_get_glyph_infos(hbBuffer, &glyphCount));
        positions.push_back(
            hb_buffer_get_glyph_positions(hbBuffer, &glyphCount));
        glyphCounts.push_back(glyphCount);

        hbBuffers.push_back(hbBuffer);
        spanCount++;

        moveDown = std::max(moveDown, span.fontSize);
    }

    calculateSize();
}

void TextElementRender::calculateSize() {
    int curX = 0;
    int curY = 0;

    TextSpans &spans = text.get();
    for (int si = 0; si < spanCount; si++) {
        TextSpan &span = spans.spans[si];
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
        if (span.newLine) {
            curX = 0;
            curY += moveDown; // this is wrong
        }
    }

    if (minX == INT_MAX) {
        minX = 0;
        minY = 0;
        maxX = 1;
        maxY = 1;
    }
}

bool TextElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();

    int curX = 0;
    int curY = 0;

    TextSpans &spans = text.get();
    for (int si = 0; si < spanCount; si++) {
        TextSpan &span = spans.spans[si];
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
                    case FT_PIXEL_MODE_MONO: {
                        if ((glyph->bitmap
                                 .buffer[y * glyph->bitmap.pitch + (x / 8)] >>
                             (7 - (x % 8))) &
                            1) {
                            Color c = getBrushPixel(span.fill, targetX, targetY,
                                                    rect.w, rect.h);
                            target[index] = over(target[index],
                                                 makePixel(c.r, c.g, c.b, c.a));
                        }
                        break;
                    }
                    case FT_PIXEL_MODE_GRAY: {
                        Color c = getBrushPixel(span.fill, targetX, targetY,
                                                rect.w, rect.h);
                        target[index] = over(
                            target[index],
                            makePixel(
                                c.r, c.g, c.b,
                                c.a *
                                    (glyph->bitmap
                                         .buffer[y * glyph->bitmap.pitch + x] /
                                     255.)));
                        break;
                    }
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
        if (span.newLine) {
            curX = 0;
            curY += moveDown; // this is wrong
        }
    }

    return true;
}

TextElementRender::~TextElementRender() {
    for (auto buffer : hbBuffers) {
        hb_buffer_destroy(buffer);
    }
}
