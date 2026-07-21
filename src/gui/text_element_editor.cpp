#include "text_element_editor.hpp"
#include <QPainter>
#include <QString>
#include <QWidget>

TextElementEditor::TextElementEditor(Scene *scene, TextElement *textElement,
                                     QObject *parent)
    : QObject(parent), textElement(textElement), scene(scene) {
    fontManager = new FontManager();
    relayout();
}

void TextElementEditor::relayout() {
    TextSpans spans = textElement->text.get({scene->currentFrame});
    spanRects.clear();

    int curX = 0;
    int curY = 0;

    for (auto &span : spans.spans) {
        FontInfo *fontInfo =
            fontManager->getFont(span.font, std::max(1, span.fontSize));

        hb_buffer_t *hbBuffer = hb_buffer_create();
        hb_buffer_add_utf8(hbBuffer, qPrintable(span.text), -1, 0, -1);

        hb_buffer_guess_segment_properties(hbBuffer);

        hb_shape(fontInfo->hb, hbBuffer, nullptr, 0);

        uint32_t glyphCount = 0;

        hb_glyph_position_t *positions =
            hb_buffer_get_glyph_positions(hbBuffer, &glyphCount);

        int minX = INT32_MAX;
        int minY = INT32_MAX;
        int maxX = INT32_MIN;
        int maxY = INT32_MIN;

        for (uint32_t i = 0; i < glyphCount; i++) {
            auto glyphPos = positions[i];
            int xOffset = glyphPos.x_offset >> 6;
            int yOffset = glyphPos.y_offset >> 6;
            int xAdvance = glyphPos.x_advance >> 6;
            int yAdvance = glyphPos.y_advance >> 6;

            int drawX = curX + xOffset;
            int drawY = curY + yOffset;

            minX = std::min(minX, drawX);
            minY = std::min(minY, drawY);
            maxX = std::max(maxX, drawX + xAdvance);
            maxY = std::max(maxY, drawY + span.fontSize);

            curX += xAdvance;
            curY += yAdvance;
        }

        if (glyphCount == 0) {
            minX = curX;
            minY = curY;
            maxX = minX + 1;
            maxY = maxY + 1;
        }

        spanRects.append({minX, minY, maxX - minX, maxY - minY});

        hb_buffer_destroy(hbBuffer);
    }
    fontManager->garbageCollect();
    ((QWidget *)parent())->update();
}

void TextElementEditor::paint(QPainter &painter) {
    float scale = painter.transform().m11();
    TextSpans spans = textElement->text.get({scene->currentFrame});

    if (spanRects.length() != spans.spans.length()) {
        return;
    }

    QRect selectionStartRect;

    qInfo() << selectionStart;

    if (selectionStart >= spanRects.length()) {
        selectionStartRect = {0, 0, 1, 1};
    } else {
        selectionStartRect = spanRects[selectionStart];
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 0, 0, 255));
    painter.drawRect(QRectF(selectionStartRect.x(), selectionStartRect.y(),
                            2. / scale, selectionStartRect.height()));
}

void TextElementEditor::passKeyEvent(QKeyEvent *keyEvent) {
    if (keyEvent->modifiers() & Qt::KeyboardModifier::ControlModifier)
        return;

    QString addedText = keyEvent->text();
    TextSpans spans = textElement->text.get({scene->currentFrame});

    if (selectionLength > 0) {
        if (keyEvent->key() == Qt::Key_Delete ||
            keyEvent->key() == Qt::Key_Backspace) {
            // TODO: selection
            return;
        }
    }

    if (keyEvent->key() == Qt::Key_Delete) {
        if (selectionStart == spans.spans.length()) {
            return;
        }

        if (spans.spans.length() == 1) {
            spans.spans[0].text = "";
        } else {
            spans.spans.removeAt(selectionStart);
        }

        textElement->text.set(spans, {scene->currentFrame});
        relayout();
        return;
    }

    if (keyEvent->key() == Qt::Key_Backspace) {
        if (selectionStart == 0) {
            return;
        }

        if (spans.spans.length() == 1) {
            spans.spans[0].text = "";
        } else {
            spans.spans.removeAt(selectionStart - 1);
        }
        selectionStart--;

        textElement->text.set(spans, {scene->currentFrame});
        relayout();
        return;
    }

    TextSpan span = spans.spans[selectionStart];

    if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
        addedText = " ";
        span.newLine = true;
    }

    if (addedText.isEmpty())
        return;

    qInfo() << "adding" << addedText;
    span.text = addedText;
    spans.spans.insert(selectionStart, span);

    selectionLength = 0;
    selectionStart += 1;

    textElement->text.set(spans, {scene->currentFrame});
    relayout();
}

TextElementEditor::~TextElementEditor() { delete fontManager; }
