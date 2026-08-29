#include "text_element.hpp"
#include "animatable/property.hpp"
#include "brush.hpp"
#include "math.hpp"
#include "render.hpp"

double TextAnimatorSelectorRender::percent(int character, int totalCharacters,
                                           int word, int totalWords, int line,
                                           int totalLines) {
    double start = this->start.get() / 100.;
    double end = this->end.get() / 100.;
    double offset = this->offset.get() / 100.;
    start += offset;
    end += offset;

    if (start > end)
        std::swap(start, end);

    int index = 0;
    int total = 1;

    BasedOn basedOn = (BasedOn)(this->basedOn.get());

    switch (basedOn) {
    case Characters: {
        index = character;
        total = std::max(totalCharacters, 1);
        break;
    }
    case Words: {
        index = word;
        total = std::max(totalWords, 1);
        break;
    }
    case Lines: {
        index = line;
        total = std::max(totalLines, 1);
        break;
    }
    }

    double thisValue = (double)index / total;
    double finalValue = 0;

    Shape shape = (Shape)(this->shape.get());

    switch (shape) {
    case Shape::Up:
    case Shape::Down: {
        // TODO: when start is 0, offset is 0, and you move end closer to zero,
        // then the first letter jumps instead of being smooth...
        if (thisValue < start) {
            finalValue = 0;
        } else if (thisValue > end) {
            finalValue = 1;
        } else {
            if ((end - start) == 0) {
                finalValue = 1;
            } else {
                finalValue = (thisValue - start) / (end - start);
            }
        }
        if (shape == Shape::Down) {
            finalValue = 1 - finalValue;
        }
        break;
    }
    case Shape::Square: {
        // TODO: smoothing on edges
        if (thisValue < start) {
            finalValue = 0;
        } else if (thisValue > end) {
            finalValue = 0;
        } else {
            finalValue = 1;
        }
        break;
    }
    }

    finalValue = easing.value.toFunction()(finalValue);

    return finalValue;
}

TextAnimatorSelector::TextAnimatorSelector(TextAnimator *textAnimator)
    : textAnimator(textAnimator) {
    start.setMin(0);
    start.setMax(100);
    end.setMin(0);
    end.setMax(100);
    offset.setMin(-100);
    offset.setMax(100);
    start.suffix = "%";
    end.suffix = "%";
    offset.suffix = "%";

    shape.enumList.push_back("Up");
    shape.enumList.push_back("Down");
    shape.enumList.push_back("Square");
    shape.updateBoundsToEnumList();

    basedOn.enumList.push_back("Characters");
    basedOn.enumList.push_back("Words");
    basedOn.enumList.push_back("Lines");
    basedOn.updateBoundsToEnumList();
}

AnimatableRender *TextAnimatorSelector::createClass() {
    return new TextAnimatorSelectorRender();
}

QString TextAnimatorSelector::displayName() { return "Selector"; }

bool TextAnimatorSelector::isCollapsed() { return collapsed; }

void TextAnimatorSelector::setCollapsed(bool newValue) {
    if (collapsed == newValue)
        return;

    collapsed = newValue;
    // TODO
    // emit collapsedChanged(collapsed);
}

void TextAnimatorSelector::_propertyUpdated(PropertyBase *property) {
    Animatable::_propertyUpdated(property);
    textAnimator->_propertyUpdated(property);
}

void TextAnimatorSelector::_propertyIsAnimatingUpdated(PropertyBase *property) {
    Animatable::_propertyIsAnimatingUpdated(property);
    textAnimator->_propertyIsAnimatingUpdated(property);
}

TextAnimatorRender::~TextAnimatorRender() { qDeleteAll(selectors); }

TextAnimator::TextAnimator(TextElement *textElement)
    : textElement(textElement) {
    opacity.setMin(0);
    opacity.setMax(100);
    opacity.suffix = "%";
}

AnimatableRender *TextAnimator::createClass() {
    return new TextAnimatorRender();
}

AnimatableRender *TextAnimator::toRender(const FrameInfo &frameInfo) {
    TextAnimatorRender *render =
        (TextAnimatorRender *)Animatable::toRender(frameInfo);
    for (auto selector : selectors) {
        render->selectors.append(
            (TextAnimatorSelectorRender *)selector->toRender(frameInfo));
    }
    return render;
}

void TextAnimator::_propertyUpdated(PropertyBase *property) {
    Animatable::_propertyUpdated(property);
    textElement->_propertyUpdated(property);
}

void TextAnimator::_propertyIsAnimatingUpdated(PropertyBase *property) {
    Animatable::_propertyIsAnimatingUpdated(property);
    textElement->_propertyIsAnimatingUpdated(property);
}

QString TextAnimator::displayName() { return "Text Animator"; }

bool TextAnimator::isCollapsed() { return collapsed; }

void TextAnimator::setCollapsed(bool newValue) {
    if (collapsed == newValue)
        return;

    collapsed = newValue;
    // TODO
    // emit collapsedChanged(collapsed);
}

bool TextAnimator::isDeletable() { return true; }
void TextAnimator::deleteThis() {
    textElement->textAnimators.removeOne(this);
    emit textElement->effectListUpdated(); // hack
    delete this;
}

QJsonObject TextAnimator::serialize() {
    QJsonObject obj = Animatable::serialize();
    QJsonArray selectorsArray;
    for (auto selector : selectors) {
        selectorsArray.append(selector->serialize());
    }
    obj["selectors"] = selectorsArray;
    return obj;
}

void TextAnimator::deserialize(const QJsonObject &obj) {
    Animatable::deserialize(obj);
    for (auto selectorJson : obj["selectors"].toArray()) {
        QJsonObject selectorObject = selectorJson.toObject();
        TextAnimatorSelector *selector = new TextAnimatorSelector(this);
        selector->deserialize(selectorObject);
        selectors.append(selector);
    }
}

TextAnimator::~TextAnimator() { qDeleteAll(selectors); }

TextLayout layoutText(FontManager *fontManager, const TextSpans &spans,
                      int width,
                      const QList<TextAnimatorRender *> &textAnimators,
                      int alignment) {
    TextLayout layout;

    int maxLineHeight = 0;
    int totalWords = 0;
    for (auto &span : spans.spans) {
        if (span.text == " ")
            totalWords++;

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
    double curX = 0;
    double curY = 0;

    int minX = INT32_MAX;
    int minY = INT32_MAX;
    int maxX = INT32_MIN;
    int maxY = INT32_MIN;

    int line = 0;
    int character = 0;
    int word = 0;
    QList<int> lineWidths;
    for (auto &span : spans.spans) {
        TextLayoutItem item;
        double offsetX = 0;
        double offsetY = 0;
        for (auto animator : textAnimators) {
            double percent = 0;
            for (auto selector : animator->selectors) {
                double selectorPercent = selector->percent(
                    character, spans.spans.size(), word, totalWords, line,
                    layout.lineHeights.size());

                // TODO: modes
                percent += selectorPercent;
                if (percent > 1)
                    percent = 1;
            }
            offsetX += animator->x * percent;
            offsetY += animator->y * percent;
            item.opacity = std::clamp(
                lerp(item.opacity, animator->opacity.get() / 100., percent), 0.,
                1.);
            item.strokeWidth += animator->strokeWidth * percent;
            item.fontSizeOffset += animator->fontSize * percent;
            curX += animator->letterSpacing * percent;
        }
        item.line = line;
        item.startPoint = {(int)(curX + offsetX), (int)(curY + offsetY)};
        item.height = span.fontSize;

        if (span.text == " ")
            word++;

        if (span.newLine) {
            lineWidths.append(curX);
            item.selectionEndPoint = {(int)(curX + (span.fontSize * .3)),
                                      (int)curY};
            curX = 0;
            curY += layout.lineHeights[line + 1];
            item.endPoint = {(int)curX, (int)curY};
            line++;
            character++;
            layout.items.append(item);
            continue;
        }

        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, qUtf8Printable(span.text), -1, 0, -1);

        hb_buffer_guess_segment_properties(hbBuffer);

        FontInfo *fontInfo = fontManager->getFont(
            span.font, std::max(1, span.fontSize), span.antialiased, {});
        if (!fontInfo)
            continue;

        hb_shape(fontInfo->hb, hbBuffer, nullptr, 0);

        uint32_t glyphCount;
        hb_glyph_position_t *positions =
            hb_buffer_get_glyph_positions(hbBuffer, &glyphCount);

        int advanced = 0;
        for (unsigned int i = 0; i < glyphCount; i++) {
            auto glyphPos = positions[i];
            double xAdvance = glyphPos.x_advance / 64.;
            double yAdvance = glyphPos.y_advance / 64.;

            minX = std::min(minX, (int)curX);
            minY = std::min(minY, (int)curY);
            maxX = std::max(maxX, (int)std::ceil(curX + xAdvance));
            maxY =
                std::max(maxY, (int)std::ceil(curY + yAdvance + span.fontSize));

            advanced += xAdvance;
            curX += xAdvance;
            curY += yAdvance;
        }
        item.advanced = advanced;

        hb_buffer_destroy(hbBuffer);

        if (width != 1 && curX > width) {
            curX -= item.startPoint.x();
            item.startPoint.setX(0);
            item.startPoint += {0, layout.lineHeights[line]};
            curY += layout.lineHeights[line];
        }

        item.endPoint = {(int)(curX + offsetX), (int)(curY + offsetY)};
        item.selectionEndPoint = item.endPoint;
        layout.items.append(item);
        character++;
    }
    lineWidths.append(curX);

    if (alignment != 0) {
        int toDivide = (alignment == 1 ? 2 : 1);
        for (auto &item : layout.items) {
            int offsetX = lineWidths[item.line] / toDivide;
            item.startPoint -= QPoint{offsetX, 0};
            if (item.endPoint.y() == item.startPoint.y()) {
                item.endPoint -= QPoint{offsetX, 0};
            } else {
                item.endPoint -=
                    QPoint{lineWidths[std::min(item.line + 1,
                                               (int)lineWidths.length() - 1)] /
                               toDivide,
                           0};
            }
            item.selectionEndPoint -= QPoint{offsetX, 0};
        }
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

    text.flags.append("textElementText");

    alignment.enumList.push_back("Left");
    alignment.enumList.push_back("Center");
    alignment.enumList.push_back("Right");
    alignment.updateBoundsToEnumList();

    TextSpans &spans = ((Keyframe<TextSpans> *)(text.keyframes[0]))->value;
    std::string defaultText = "Enter text";
    for (auto character : defaultText) {
        TextSpan defaultSpan = createDefaultTextSpan();
        defaultSpan.text = character;
        spans.spans.append(std::move(defaultSpan));
    }
}

TextElement::~TextElement() {
    delete fontManager;
    qDeleteAll(textAnimators);
}

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
    auto animators = toRenderAnimators(frameInfo);
    TextLayout lay = layoutText(fontManager, spans, w.get(frameInfo), animators,
                                alignment.get(frameInfo));
    qDeleteAll(animators);
    return lay;
}

QRect TextElement::getRawBoundingBox(const FrameInfo &frameInfo) {
    if (w.get(frameInfo) != 1 || h.get(frameInfo) != 1) {
        return Element::getRawBoundingBox(frameInfo);
    }

    TextLayout layout = layTheTextOut(frameInfo);

    TextSpans spans = text.get(frameInfo);

    fontManager->garbageCollect();
    int offsetX = 0;
    int alignment = this->alignment.get(frameInfo);
    if (alignment == 1) {
        offsetX = layout.width / -2;
    } else if (alignment == 2) {
        offsetX = -layout.width;
    }

    return {x.get(frameInfo) + offsetX,
            y.get(frameInfo) - layout.lineHeights[0], layout.width,
            layout.height};
}

AnimatableRender *TextElement::createClass() { return new TextElementRender(); }

AnimatableRender *TextElement::toRender(const FrameInfo &frameInfo) {
    TextElementRender *render =
        (TextElementRender *)Element::toRender(frameInfo);
    render->textAnimators = toRenderAnimators(frameInfo);
    return render;
}

QList<TextAnimatorRender *>
TextElement::toRenderAnimators(const FrameInfo &frameInfo) {
    QList<TextAnimatorRender *> newList;
    for (auto animator : textAnimators) {
        newList.append((TextAnimatorRender *)animator->toRender(frameInfo));
    }
    return newList;
}

QJsonObject TextElement::serialize() {
    QJsonObject obj = Element::serialize();
    QJsonArray textAnimatorsArray;
    for (auto animator : textAnimators) {
        textAnimatorsArray.append(animator->serialize());
    }
    obj["textAnimators"] = textAnimatorsArray;
    return obj;
}

void TextElement::deserialize(const QJsonObject &obj) {
    Element::deserialize(obj);
    for (auto animatorJson : obj["textAnimators"].toArray()) {
        QJsonObject animatorObject = animatorJson.toObject();
        TextAnimator *animator = new TextAnimator(this);
        animator->deserialize(animatorObject);
        textAnimators.append(animator);
    }
}

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
    layout = layoutText(renderThread->fontManager, text, w, textAnimators,
                        alignment.get());

    for (auto span : text.get().spans) {
        TextLayoutItem &item = layout.items[spanCount];

        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, qUtf8Printable(span.text), -1, 0, -1);

        hb_buffer_guess_segment_properties(hbBuffer);

        int fontSize = std::max(1, span.fontSize + item.fontSizeOffset);
        double multiplier = 1 - ((double)fontSize / span.fontSize);
        item.startPoint.setX(item.startPoint.x() +
                             item.advanced * multiplier / 2);

        FontInfo *fontInfo = renderThread->fontManager->getFont(
            span.font, fontSize, span.antialiased, {});
        if (!fontInfo)
            continue;
        fontInfos.push_back(fontInfo);

        span.strokeWidth += item.strokeWidth;

        if (span.strokeWidth > 0) {
            FontInfo *strokeFontInfo = renderThread->fontManager->getFont(
                span.font, fontSize, span.antialiased, span.strokeInfo());
            if (strokeFontInfo) {
                strokeFontInfos.push_back(strokeFontInfo);
            }
        } else {
            strokeFontInfos.push_back(nullptr);
        }

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
            FontInfo *fontInfo = strokeFontInfos[si];
            if (fontInfo == nullptr) {
                fontInfo = fontInfos[si];
            }
            auto glyph = fontInfo->getGlyph(codepoint);

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

void TextElementRender::renderGlyph(uint32_t *target, FT_BitmapGlyph glyph,
                                    int x, int y, Rect &rect, Brush &brush,
                                    double opacity) {
    int drawX = x + glyph->left;
    int drawY = y - glyph->top;
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
                if ((glyph->bitmap.buffer[y * glyph->bitmap.pitch + (x / 8)] >>
                     (7 - (x % 8))) &
                    1) {
                    Color c =
                        getBrushPixel(brush, targetX, targetY, rect.w, rect.h);
                    target[index] = over(
                        target[index], makePixel(c.r, c.g, c.b, c.a * opacity));
                }
                break;
            }
            case FT_PIXEL_MODE_GRAY: {
                Color c =
                    getBrushPixel(brush, targetX, targetY, rect.w, rect.h);
                target[index] = over(
                    target[index],
                    makePixel(
                        c.r, c.g, c.b,
                        opacity * c.a *
                            (glyph->bitmap.buffer[y * glyph->bitmap.pitch + x] /
                             255.)));
                break;
            }
            case FT_PIXEL_MODE_BGRA: {
                uint8_t *bitmapPixel =
                    &(glyph->bitmap.buffer)[(y * glyph->bitmap.width + x) * 4];
                uint8_t alpha = bitmapPixel[3] * opacity;
                uint8_t red = bitmapPixel[2];
                uint8_t green = bitmapPixel[1];
                uint8_t blue = bitmapPixel[0];
                target[index] =
                    over(target[index], makePixel(red, green, blue, alpha));
                break;
            }
            default: {
                invalid = true;
                target[index] = over(target[index], makePixel(255, 0, 0, 255));
            }
            }
        }
    }

    if (invalid) {
        qInfo() << "Invalid pixel_mode" << glyph->bitmap.pixel_mode;
    }
}

bool TextElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();

    TextSpans &spans = text.get();
    // stroke
    for (int si = 0; si < spanCount; si++) {
        TextSpan &span = spans.spans[si];
        TextLayoutItem &item = layout.items[si];
        int curX = item.startPoint.x();
        int curY = item.startPoint.y();

        for (unsigned int i = 0; i < glyphCounts[si]; i++) {
            auto glyphPos = positions[si][i];
            auto codepoint = infos[si][i].codepoint;
            FontInfo *strokeFontInfo = strokeFontInfos[si];
            if (!strokeFontInfo)
                continue;
            if (strokeFontInfo->pixelHeight <= 1)
                continue;

            int xOffset = (glyphPos.x_offset >> 6);
            int yOffset = (glyphPos.y_offset >> 6);

            int itemX = (curX + xOffset) - minX;
            int itemY = (curY + yOffset) - minY;

            auto glyph = strokeFontInfo->getGlyph(infos[si][i].codepoint);
            renderGlyph(target, glyph, itemX, itemY, rect, span.stroke,
                        item.opacity);
        }
    }

    // fill
    for (int si = 0; si < spanCount; si++) {
        TextSpan &span = spans.spans[si];
        TextLayoutItem &item = layout.items[si];
        int curX = item.startPoint.x();
        int curY = item.startPoint.y();

        for (unsigned int i = 0; i < glyphCounts[si]; i++) {
            auto glyphPos = positions[si][i];
            auto codepoint = infos[si][i].codepoint;
            FontInfo *fontInfo = fontInfos[si];
            if (fontInfo->pixelHeight <= 1)
                continue;

            auto glyph = fontInfo->getGlyph(infos[si][i].codepoint);

            int xOffset = (glyphPos.x_offset >> 6);
            int yOffset = (glyphPos.y_offset >> 6);

            int itemX = (curX + xOffset) - minX;
            int itemY = (curY + yOffset) - minY;

            renderGlyph(target, glyph, itemX, itemY, rect, span.fill,
                        item.opacity);
        }
    }

    return true;
}

TextElementRender::~TextElementRender() {
    for (auto buffer : hbBuffers) {
        hb_buffer_destroy(buffer);
    }
    qDeleteAll(textAnimators);
}
