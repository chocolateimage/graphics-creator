#include "text_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"
#include "render.hpp"

TextLayout layoutText(FontManager *fontManager, const TextSpans &spans,
                      int width) {
    TextLayout layout;

    int maxLineHeight = 0;
    for (auto &span : spans.spans) {
        if (span.newLine) {
            if (maxLineHeight == 0) {
                maxLineHeight = span.fontSize;
            }
            layout.lineHeights.append(maxLineHeight);
            maxLineHeight = 0;
            continue;
        }
        maxLineHeight = std::max(maxLineHeight, span.fontSize);
    }
    if (maxLineHeight == 0 && !spans.spans.isEmpty()) {
        maxLineHeight = spans.spans.last().fontSize;
    }
    layout.lineHeights.append(maxLineHeight);

    // 0x0 is actually not top left corner of the rendered box, but instead it's
    // the bottom left of the first line
    int curX = 0;
    int curY = 0;

    int minX = INT32_MAX;
    int minY = INT32_MAX;
    int maxX = INT32_MIN;
    int maxY = INT32_MIN;

    int line = 0;
    for (auto &span : spans.spans) {
        TextLayoutItem item;
        item.line = line;
        item.startPoint = {curX, curY};
        item.height = span.fontSize;

        if (span.newLine) {
            item.selectionEndPoint = {curX + (int)(span.fontSize * .3), curY};
            curX = 0;
            curY += layout.lineHeights[line + 1];
            item.endPoint = {curX, curY};
            line++;
            layout.items.append(item);
            continue;
        }

        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, qUtf8Printable(span.text), -1, 0, -1);

        hb_buffer_guess_segment_properties(hbBuffer);

        FontInfo *fontInfo =
            fontManager->getFont(span.font, std::max(1, span.fontSize));

        hb_shape(fontInfo->hb, hbBuffer, nullptr, 0);

        uint32_t glyphCount;
        hb_glyph_position_t *positions =
            hb_buffer_get_glyph_positions(hbBuffer, &glyphCount);

        for (unsigned int i = 0; i < glyphCount; i++) {
            auto glyphPos = positions[i];
            int xOffset = glyphPos.x_offset >> 6;
            int yOffset = glyphPos.y_offset >> 6;
            int xAdvance = glyphPos.x_advance >> 6;
            int yAdvance = glyphPos.y_advance >> 6;

            minX = std::min(minX, curX);
            minY = std::min(minY, curY);
            maxX = std::max(maxX, curX + xAdvance);
            maxY = std::max(maxY, curY + yAdvance + span.fontSize);

            curX += xAdvance;
            curY += yAdvance;
        }

        hb_buffer_destroy(hbBuffer);

        if (width != 1 && curX >= width) {
            curX -= item.startPoint.x();
            item.startPoint.setX(0);
            item.startPoint += {0, layout.lineHeights[line]};
            curY += layout.lineHeights[line];
        }

        item.endPoint = {curX, curY};
        item.selectionEndPoint = item.endPoint;
        layout.items.append(item);
    }

    layout.width = maxX - minX;
    layout.height = 0;
    for (auto height : layout.lineHeights) {
        layout.height += height;
    }

    fontManager->garbageCollect();
    return layout;
}

TextElement::TextElement() : Element() {
    fontManager = new FontManager();
    TextSpans &spans = ((Keyframe<TextSpans> *)(text.keyframes[0]))->value;
    std::string defaultText = "Hello world";
    for (auto character : defaultText) {
        TextSpan defaultSpan = createDefaultTextSpan();
        defaultSpan.text = character;
        spans.spans.append(std::move(defaultSpan));
    }
}

TextElement::~TextElement() { delete fontManager; }

TextSpan TextElement::createDefaultTextSpan() {
    TextSpan defaultSpan;
    defaultSpan.text = "";
    defaultSpan.fill = {};
    defaultSpan.font = Variant::defaultFont;
    defaultSpan.fontSize = 128;
    return defaultSpan;
}

TextLayout TextElement::layTheTextOut(const FrameInfo &frameInfo) {
    TextSpans spans = text.get(frameInfo);

    return layoutText(fontManager, spans, w.get(frameInfo));
}

QRect TextElement::getBoundingBox(const FrameInfo &frameInfo) {
    if (w.get(frameInfo) != 1 || h.get(frameInfo) != 1) {
        return Element::getBoundingBox(frameInfo);
    }

    TextLayout layout = layTheTextOut(frameInfo);

    TextSpans spans = text.get(frameInfo);

    fontManager->garbageCollect();
    return {x.get(frameInfo), y.get(frameInfo) - layout.lineHeights[0],
            layout.width, layout.height};
}

AnimatableRender *TextElement::createClass() { return new TextElementRender(); }

Rect TextElementRender::getRenderBox() {
    int yOffset = 0;
    int height = maxY - minY;
    if (w.get() != 1 || h.get() != 1) {
        yOffset += layout.lineHeights[0];
        height = h - (layout.lineHeights[0] + minY);
    }
    return {minX + x, minY + y + yOffset, std::max(1, maxX - minX),
            std::max(1, height)};
}

void TextElementRender::prepare() {
    layout = layoutText(renderThread->fontManager, text, w);

    for (auto span : text.get().spans) {
        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, qUtf8Printable(span.text), -1, 0, -1);

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
    }

    calculateSize();
}

void TextElementRender::calculateSize() {
    for (int si = 0; si < spanCount; si++) {
        TextLayoutItem &item = layout.items[si];
        int curX = item.startPoint.x();
        int curY = item.startPoint.y();
        for (unsigned int i = 0; i < glyphCounts[si]; i++) {
            auto codepoint = infos[si][i].codepoint;
            auto glyph = fontInfos[si]->getGlyph(codepoint);

            auto glyphPos = positions[si][i];
            int xOffset = glyphPos.x_offset >> 6;
            int yOffset = glyphPos.y_offset >> 6;

            int drawX = (curX + xOffset + glyph->left);
            int drawY = (curY + yOffset - glyph->top);

            minX = std::min(minX, drawX);
            minY = std::min(minY, drawY);
            maxX = std::max(maxX, (int)glyph->bitmap.width + drawX);
            maxY = std::max(maxY, (int)glyph->bitmap.rows + drawY);
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

    TextSpans &spans = text.get();
    for (int si = 0; si < spanCount; si++) {
        TextSpan &span = spans.spans[si];
        TextLayoutItem &item = layout.items[si];
        int curX = item.startPoint.x();
        int curY = item.startPoint.y();
        for (unsigned int i = 0; i < glyphCounts[si]; i++) {
            auto glyphPos = positions[si][i];
            auto codepoint = infos[si][i].codepoint;
            auto glyph = fontInfos[si]->getGlyph(infos[si][i].codepoint);

            int xOffset = (glyphPos.x_offset >> 6);
            int yOffset = (glyphPos.y_offset >> 6);

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
        }
    }

    return true;
}

TextElementRender::~TextElementRender() {
    for (auto buffer : hbBuffers) {
        hb_buffer_destroy(buffer);
    }
}
