#include "text_element_editor.hpp"
#include "draggable_spinbox.hpp"
#include "gui.hpp"
#include <QPainter>
#include <QString>
#include <QWidget>

TextElementEditor::TextElementEditor(NewMainWindow *mainWindow, Scene *scene,
                                     TextElement *textElement, QObject *parent)
    : QObject(parent), textElement(textElement), scene(scene),
      mainWindow(mainWindow) {
    selectionLength =
        textElement->text.get({scene->currentFrame}).spans.length();
    dockContentWidget = new QWidget();
    QFormLayout *layout = new QFormLayout(dockContentWidget);

    fontSize = new DraggableSpinBox(dockContentWidget);
    fontSize->setRange(1, 500);
    connect(fontSize, &QSpinBox::valueChanged, this,
            &TextElementEditor::setFontSize);
    layout->addRow("Font size", fontSize);

    debugListWidget = new QListWidget(dockContentWidget);
    layout->addWidget(debugListWidget);

    dockWidget = mainWindow->dockManager->createDockWidget("Text");
    dockWidget->setWidget(dockContentWidget);
    dockWidget->setIcon(QIcon::fromTheme("draw-text"));
    dockWidget->setFeature(
        ads::CDockWidget::DockWidgetFeature::DockWidgetClosable, false);
    mainWindow->dockManager->addDockWidgetTabToArea(
        dockWidget, mainWindow->propertiesDockWidget->dockAreaWidget());
    relayout();
}

void TextElementEditor::loadValues() {
    TextSpans spans = textElement->text.get({scene->currentFrame});

    // TODO: "new" writing: when length is zero and you are about to type. can
    // also happen when text is empty
    dockContentWidget->setDisabled(spans.spans.isEmpty());
    if (spans.spans.isEmpty()) {
        return;
    }

    // TODO: multiple values
    QSignalBlocker blocker{fontSize};
    int index = selectionStart;
    if (index >= spans.spans.length()) {
        index = spans.spans.length() - 1;
    }
    if (index < 0) {
        index = 0;
    }
    TextSpan &span = spans.spans[index];
    fontSize->setValue(span.fontSize);
}

void TextElementEditor::setFontSize(int newValue) {
    TextSpans spans = textElement->text.get({scene->currentFrame});
    for (int i = selectionStart;
         i < selectionStart + std::max(1, selectionLength); i++) {
        spans.spans[i].fontSize = newValue;
    }
    textElement->text.set(spans, {scene->currentFrame});
    relayout();
}

void TextElementEditor::relayout() {
    layout = textElement->layTheTextOut({scene->currentFrame});
    debugListWidget->clear();
    TextSpans spans = textElement->text.get({scene->currentFrame});
    int index = 0;
    for (const auto &item : layout.items) {
        debugListWidget->addItem(QString::number(index) + ": [" +
                                 spans.spans[index].text + "] " +
                                 QString::number(item.startPoint.x()) + "x" +
                                 QString::number(item.startPoint.y()));
        index++;
    }

    ((QWidget *)parent())->update();
    loadValues();
}

void TextElementEditor::paint(QPainter &painter) {
    QWidget *parentWidget = ((QWidget *)parent());
    float scale = painter.transform().m11();
    TextSpans spans = textElement->text.get({scene->currentFrame});

    if (layout.items.length() != spans.spans.length()) {
        qWarning()
            << "what is going on. layout.items.length != spans.spans.length";
        return;
    }

    painter.translate(0, layout.lineHeights[0]);
    qInfo() << "LINE HEIGHTS" << layout.lineHeights.length();
    for (auto a : layout.lineHeights) {
        qInfo() << "\t" << a;
    }
    if (selectionLength == 0) {
        QPoint cursorPoint = {0, 0};
        int height = 128;
        if (selectionStart > 0) {
            auto &item = layout.items[selectionStart - 1];
            qInfo() << "selection start:" << selectionStart << "is on line"
                    << item.line << "| START" << item.startPoint << " END "
                    << item.endPoint;
            cursorPoint = item.endPoint;
            height = item.height;
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 0, 0, 255));
        painter.drawRect(QRectF(cursorPoint.x(), cursorPoint.y() - height,
                                2. / scale, height));
    } else {
        for (int i = selectionStart; i < selectionStart + selectionLength;
             i++) {
            auto &item = layout.items[i];
            QPoint selectionStartRect = item.startPoint;
            QRect rect = {selectionStartRect.x(),
                          selectionStartRect.y() - item.height,
                          item.selectionEndPoint.x() - selectionStartRect.x(),
                          item.height};
            painter.setPen(Qt::NoPen);
            QColor color = parentWidget->palette().highlight().color();
            color.setAlpha(120);
            painter.setBrush(color);
            painter.drawRect(rect);
        }
    }

    // for (auto r : spanRects) {
    //     QColor color = Qt::green;
    //     painter.setPen(color);
    //     color.setAlpha(30);
    //     painter.setBrush(color);
    //     painter.drawRect(r);
    // }
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

TextElementEditor::~TextElementEditor() { dockWidget->deleteDockWidget(); }
