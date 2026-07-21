#include "text_element_editor.hpp"
#include <QPainter>
#include <QString>
#include <QWidget>

TextElementEditor::TextElementEditor(Scene *scene, TextElement *textElement,
                                     QObject *parent)
    : QObject(parent), textElement(textElement), scene(scene) {
    fontManager = new FontManager();
    selectionLength =
        textElement->text.get({scene->currentFrame}).spans.length();
    relayout();
}

void TextElementEditor::relayout() {
    TextSpans spans = textElement->text.get({scene->currentFrame});
    spanRects.clear();

    int curX = 0;
    int curY = 0;

    for (auto &span : spans.spans) {
        if (span.newLine) {
            curX = 0;
            curY += span.fontSize; // this is wrong
            spanRects.append({curX, curY, 1, span.fontSize});
            continue;
        }

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
            maxY = minY + 1;
        }

        spanRects.append({minX, minY, maxX - minX, maxY - minY});

        hb_buffer_destroy(hbBuffer);
    }
    fontManager->garbageCollect();
    ((QWidget *)parent())->update();
}

QRect TextElementEditor::getRectForIndex(int index) {
    TextSpans spans = textElement->text.get({scene->currentFrame});
    if (spanRects.isEmpty()) {
        return {0, 0, 1, 1};
    } else if (index >= spans.spans.length()) {
        if (index > spans.spans.length()) {
            qWarning() << "getRectForIndex: Something bad is going on!!"
                       << index << ">" << spans.spans.length();
        }
        QRect rect = spanRects.last();
        rect.setX(rect.x() + rect.width());
        return rect;
    } else {
        return spanRects[index];
    }
}

void TextElementEditor::paint(QPainter &painter) {
    QWidget *parentWidget = ((QWidget *)parent());
    float scale = painter.transform().m11();
    TextSpans spans = textElement->text.get({scene->currentFrame});

    if (spanRects.length() != spans.spans.length()) {
        return;
    }

    QRect selectionStartRect = getRectForIndex(selectionStart);
    QRect selectionEndRect = selectionStartRect;

    if (selectionLength > 0) {
        selectionEndRect = getRectForIndex(selectionStart + selectionLength);
    }

    if (selectionLength == 0) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 0, 0, 255));
        painter.drawRect(QRectF(selectionStartRect.x(), selectionStartRect.y(),
                                2. / scale, selectionStartRect.height()));
    } else {
        QColor color = parentWidget->palette().highlight().color();
        painter.setPen(color);
        color.setAlpha(120);
        painter.setBrush(color);
        painter.drawRect(QRectF(selectionStartRect.x(), selectionStartRect.y(),
                                selectionEndRect.x() - selectionStartRect.x(),
                                selectionStartRect.height()));
    }
}

void TextElementEditor::passKeyEvent(QKeyEvent *keyEvent) {

    QString addedText = keyEvent->text();
    TextSpans spans = textElement->text.get({scene->currentFrame});

    TextSpan span;
    if (selectionStart < spans.spans.length()) {
        span = spans.spans[selectionStart];
    } else if (!spans.spans.isEmpty()) {
        span = spans.spans.last();
    } else {
        span = textElement->createDefaultTextSpan();
    }

    if (selectionLength > 0) {
        if (keyEvent->key() == Qt::Key_Delete ||
            keyEvent->key() == Qt::Key_Backspace) {

            while (selectionLength > 0) {
                spans.spans.removeAt(selectionStart);
                selectionLength--;
            }

            textElement->text.set(spans, {scene->currentFrame});
            relayout();
            return;
        }
    }

    if (keyEvent == QKeySequence::SelectAll) {
        selectionStart = 0;
        selectionLength = spans.spans.length();
        relayout();
        return;
    }

    if (keyEvent->key() == Qt::Key_Left) {
        if (keyEvent->modifiers() & Qt::KeyboardModifier::ShiftModifier) {
            if (selectionLength == 0) {
                selectionAnchorLeft = true;
            }

            if (selectionAnchorLeft) {
                if (selectionStart > 0) {
                    selectionLength = selectionLength + 1;
                }
                selectionStart = std::max(0, selectionStart - 1);
            } else {
                selectionLength = selectionLength - 1;
            }
        } else {
            if (selectionLength == 0) {
                selectionStart = std::max(0, selectionStart - 1);
            } else {
                selectionLength = 0;
            }
        }
        relayout();
        return;
    }
    if (keyEvent->key() == Qt::Key_Right) {
        if (keyEvent->modifiers() & Qt::KeyboardModifier::ShiftModifier) {
            if (selectionLength == 0) {
                selectionAnchorLeft = false;
            }

            if (selectionAnchorLeft) {
                selectionLength = selectionLength - 1;
                selectionStart =
                    std::min((int)spans.spans.length(), selectionStart + 1);
            } else {
                if (selectionStart + selectionLength < spans.spans.length()) {
                    selectionLength = selectionLength + 1;
                }
            }
        } else {
            selectionStart =
                std::min((int)spans.spans.length(),
                         selectionStart + std::max(1, selectionLength));
            selectionLength = 0;
        }
        relayout();
        return;
    }

    if (keyEvent->key() == Qt::Key_Delete) {
        if (selectionStart == spans.spans.length()) {
            return;
        }

        spans.spans.removeAt(selectionStart);

        textElement->text.set(spans, {scene->currentFrame});
        relayout();
        return;
    }

    if (keyEvent->key() == Qt::Key_Backspace) {
        if (selectionStart == 0) {
            return;
        }

        spans.spans.removeAt(selectionStart - 1);
        selectionStart--;

        textElement->text.set(spans, {scene->currentFrame});
        relayout();
        return;
    }

    if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
        addedText = " ";
        span.newLine = true;
    } else {
        span.newLine = false;
    }

    if (addedText.isEmpty())
        return;

    if (keyEvent->modifiers() & Qt::KeyboardModifier::ControlModifier)
        return;

    while (selectionLength > 0) {
        spans.spans.removeAt(selectionStart);
        selectionLength--;
    }
    span.text = addedText;
    spans.spans.insert(selectionStart, span);

    selectionLength = 0;
    selectionStart += 1;

    textElement->text.set(spans, {scene->currentFrame});
    relayout();
}

TextElementEditor::~TextElementEditor() { delete fontManager; }
