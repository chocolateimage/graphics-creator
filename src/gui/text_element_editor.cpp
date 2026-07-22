#include "text_element_editor.hpp"
#include "draggable_spinbox.hpp"
#include "gui.hpp"
#include <QApplication>
#include <QClipboard>
#include <QPainter>
#include <QString>
#include <QTextBoundaryFinder>
#include <QWidget>

TextElementEditor::TextElementEditor(NewMainWindow *mainWindow, Scene *scene,
                                     TextElement *textElement, QObject *parent)
    : QObject(parent), textElement(textElement), scene(scene),
      mainWindow(mainWindow) {
    tempSpan = textElement->createDefaultTextSpan();
    selectionLength =
        textElement->text.get({scene->currentFrame}).spans.length();
    dockContentWidget = new QWidget();
    QFormLayout *layout = new QFormLayout(dockContentWidget);

    fontComboBox = new FontComboBox(dockContentWidget);
    fontComboBox->setSizePolicy(QSizePolicy::Policy::Expanding,
                                QSizePolicy::Policy::Fixed);
    connect(fontComboBox, &FontComboBox::currentTextChanged, this,
            &TextElementEditor::setFont);
    layout->addRow("Font", fontComboBox);

    fontSize = new DraggableSpinBox(dockContentWidget);
    fontSize->setRange(1, 500);
    connect(fontSize, &QSpinBox::valueChanged, this,
            &TextElementEditor::setFontSize);
    layout->addRow("Font size", fontSize);

    fillInput = new BrushInput(dockContentWidget);
    connect(fillInput, &BrushInput::valueChanged, this,
            &TextElementEditor::setFill);
    layout->addRow("Fill", fillInput);

    antialiasedCheckBox = new QCheckBox(dockContentWidget);
    connect(antialiasedCheckBox, &QCheckBox::toggled, this,
            &TextElementEditor::setAntialiased);
    layout->addRow("Antialiased", antialiasedCheckBox);

    debugListWidget = new QListWidget(dockContentWidget);
    debugListWidget->setVisible(debug);
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

void TextElementEditor::repaintParent() { ((QWidget *)parent())->update(); }

void TextElementEditor::loadValues() {
    TextSpans spans = textElement->text.get({scene->currentFrame});

    if (selectionLength == 0 && !spans.spans.isEmpty()) {
        tempSpan = spans.spans[std::max(selectionStart - 1, 0)];
    }
    if (selectionLength == 0) {
        loadValues(tempSpan);
        return;
    }

    // TODO: multiple values
    int index = selectionStart;
    if (selectionLength == 0) {
        index--;
    }
    if (index >= spans.spans.length()) {
        index = spans.spans.length() - 1;
    }
    if (index < 0) {
        index = 0;
    }
    TextSpan &span = spans.spans[index];
    loadValues(span);
}

void TextElementEditor::loadValues(TextSpan &span) {
    QSignalBlocker blocker{fontSize};
    QSignalBlocker blocker2{fontComboBox};
    QSignalBlocker blocker3{fillInput};
    QSignalBlocker blocker4{antialiasedCheckBox};
    fontSize->setValue(span.fontSize);
    fontComboBox->setFontValue(span.font);
    fillInput->setValue(span.fill);
    antialiasedCheckBox->setChecked(span.antialiased);
}

void TextElementEditor::setSpanProperties(
    std::function<void(TextSpan &)> func) {
    if (selectionLength == 0) {
        func(tempSpan);
        repaintParent();
        return;
    }

    TextSpans spans = textElement->text.get({scene->currentFrame});
    for (int i = selectionStart;
         i < selectionStart + std::max(1, selectionLength); i++) {
        func(spans.spans[i]);
    }
    textElement->text.set(spans, {scene->currentFrame});
    relayout();
}

void TextElementEditor::setFontSize(int newValue) {
    setSpanProperties([newValue](TextSpan &span) { span.fontSize = newValue; });
}

void TextElementEditor::setFont() {
    setSpanProperties([this](TextSpan &span) {
        span.font = this->fontComboBox->fontValue();
    });
}

void TextElementEditor::setFill(Brush value) {
    setSpanProperties([value](TextSpan &span) { span.fill = value; });
}

void TextElementEditor::setAntialiased(bool newValue) {
    setSpanProperties(
        [newValue](TextSpan &span) { span.antialiased = newValue; });
}

void TextElementEditor::relayout() {
    layout = textElement->layTheTextOut({scene->currentFrame});
    TextSpans spans = textElement->text.get({scene->currentFrame});

    if (debug) {
        debugListWidget->clear();
        int index = 0;
        for (const auto &item : layout.items) {
            debugListWidget->addItem(
                QString::number(index) + ": [" + spans.spans[index].text +
                "] " + QString::number(item.startPoint.x()) + "x" +
                QString::number(item.startPoint.y()));
            index++;
        }
    }

    repaintParent();
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

    if (spans.spans.isEmpty()) {
        painter.translate(0, tempSpan.fontSize);
    } else {
        painter.translate(0, layout.lineHeights[0]);
    }
    if (selectionLength == 0) {
        QPoint cursorPoint = {0, 0};
        int height = 128;
        if (selectionStart > 0) {
            auto &item = layout.items[selectionStart - 1];
            cursorPoint = item.endPoint;
            height = item.height;
        }
        if (selectionLength == 0) {
            height = tempSpan.fontSize;
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

    if (debug) {
        for (auto &item : layout.items) {
            {
                QColor color = Qt::blue;
                painter.setPen(color);
                color.setAlpha(30);
                painter.setBrush(color);
                QPolygon polygon;
                polygon << item.startPoint;
                polygon << item.startPoint + QPoint(0, -item.height);
                polygon << item.endPoint + QPoint(0, -item.height);
                polygon << item.endPoint;
                painter.drawPolygon(polygon);
            }
            {
                QColor color = Qt::red;
                painter.setPen(color);
                color.setAlpha(30);
                painter.setBrush(color);
                QPolygon polygon;
                polygon << item.startPoint;
                polygon << item.startPoint + QPoint(0, -item.height);
                polygon << item.selectionEndPoint + QPoint(0, -item.height);
                polygon << item.selectionEndPoint;
                painter.drawPolygon(polygon);
            }
        }
    }
}

void TextElementEditor::passKeyEvent(QKeyEvent *keyEvent) {
    QString addedText = keyEvent->text();
    TextSpans spans = textElement->text.get({scene->currentFrame});

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

    if (keyEvent == QKeySequence::Paste) {
        QClipboard *clipboard = QApplication::clipboard();
        QString text = clipboard->text();
        if (text.isEmpty())
            return;

        int index = 0;
        textElement->blockSignals(true);

        QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);

        QStringList list;
        int start = 0;
        while (finder.toNextBoundary() != -1) {
            int end = finder.position();
            list << text.mid(start, end - start);
            start = end;
        }

        for (auto character : list) {
            QKeyEvent *keyEvent =
                new QKeyEvent(QEvent::KeyPress, 0,
                              Qt::KeyboardModifier::NoModifier, character);
            if (index == list.length() - 1) {
                textElement->blockSignals(false);
            }
            passKeyEvent(keyEvent);
            delete keyEvent;
            index++;
        }
        return;
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

    TextSpan span;
    if (selectionLength == 0) {
        span = tempSpan;
    } else {
        span = spans.spans[selectionStart];
    }

    if (addedText == "\n" || keyEvent->key() == Qt::Key_Enter ||
        keyEvent->key() == Qt::Key_Return) {
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
