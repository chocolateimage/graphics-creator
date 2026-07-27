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

    int index = character;
    int total = std::max(totalCharacters, 1);

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
                      const QList<TextAnimatorRender *> &textAnimators) {
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
    int character = 0;
    for (auto &span : spans.spans) {
        TextLayoutItem item;
        int offsetX = 0;
        int offsetY = 0;
        for (auto animator : textAnimators) {
            double percent = 0;
            for (auto selector : animator->selectors) {
                // TODO: words
                double selectorPercent =
                    selector->percent(character, spans.spans.size(), 0, 0, line,
                                      layout.lineHeights.size());

                // TODO: modes
                percent += selectorPercent;
                if (percent > 1)
                    percent = 1;
            }
            offsetX += animator->x * percent;
            offsetY += animator->y * percent;
            item.opacity =
                lerp(item.opacity, animator->opacity.get() / 100., percent);
        }
        item.line = line;
        item.startPoint = {curX + offsetX, curY + offsetY};
        item.height = span.fontSize;

        if (span.newLine) {
            item.selectionEndPoint = {curX + (int)(span.fontSize * .3), curY};
            curX = 0;
            curY += layout.lineHeights[line + 1];
            item.endPoint = {curX, curY};
            line++;
            character++;
            character = 0;
            layout.items.append(item);
            continue;
        }

        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, qUtf8Printable(span.text), -1, 0, -1);

        hb_buffer_guess_segment_properties(hbBuffer);

        FontInfo *fontInfo = fontManager->getFont(
            span.font, std::max(1, span.fontSize), span.antialiased, {});

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

        item.endPoint = {curX + offsetX, curY + offsetY};
        item.selectionEndPoint = item.endPoint;
        layout.items.append(item);
        character++;
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
    TextLayout lay =
        layoutText(fontManager, spans, w.get(frameInfo), animators);
    qDeleteAll(animators);
    return lay;
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
    layout = layoutText(renderThread->fontManager, text, w, textAnimators);

    for (auto span : text.get().spans) {
        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, qUtf8Printable(span.text), -1, 0, -1);

        hb_buffer_guess_segment_properties(hbBuffer);

        FontInfo *fontInfo = renderThread->fontManager->getFont(
            span.font, std::max(1, span.fontSize), span.antialiased, {});
        fontInfos.push_back(fontInfo);

        if (span.strokeWidth > 0) {
            FontInfo *strokeFontInfo = renderThread->fontManager->getFont(
                span.font, std::max(1, span.fontSize), span.antialiased,
                span.strokeInfo());
            strokeFontInfos.push_back(strokeFontInfo);
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
            case FT_PIXEL_MODE_BGRA:
                // TODO: opacity
                target[index] = over(
                    target[index],
                    ((uint32_t *)(glyph->bitmap
                                      .buffer))[y * glyph->bitmap.width + x]);
                break;
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
            auto glyph = fontInfos[si]->getGlyph(infos[si][i].codepoint);

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
