#include "draggable_spinbox.hpp"
#include <QApplication>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>

DraggableSpinBox::DraggableSpinBox(QWidget *parent) : QSpinBox(parent) {
    lineEdit()->setCursor(Qt::CursorShape::SizeHorCursor);
    lineEdit()->installEventFilter(this);
}

bool DraggableSpinBox::eventFilter(QObject *obj, QEvent *event) {
    if (obj == lineEdit()) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (!isHolding) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                holdStartPos = mouseEvent->position();
                holdStartValue = value();
                isDragging = false;
                isHolding = true;
                holdCompleteStartPos = QCursor::pos();
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (isHolding) {
                float moved = mouseEvent->position().x() - holdStartPos.x();
                if (isDragging) {
                    int oldStartValue = holdStartValue;
                    if ((QCursor::pos() - holdAfterDragStartPos)
                            .manhattanLength() > 100) {
                        holdStartValue = value();
                        QCursor::setPos(holdAfterDragStartPos);
                    }
                    setValue(oldStartValue + moved);
                    return true;
                } else {
                    if (qAbs(moved) >= QApplication::startDragDistance()) {
                        holdAfterDragStartPos = QCursor::pos();
                        holdStartPos = mouseEvent->position();
                        isDragging = true;
                        QApplication::setOverrideCursor(
                            Qt::CursorShape::BlankCursor);
                    }
                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease ||
                   event->type() == QEvent::ContextMenu) {
            if (isDragging) {
                isDragging = false;
                QCursor::setPos(holdCompleteStartPos);
                QApplication::restoreOverrideCursor();
                clearFocus();
            }
            isHolding = false;
        }
    }
    return QObject::eventFilter(obj, event);
}

DraggableDoubleSpinBox::DraggableDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent) {
    lineEdit()->setCursor(Qt::CursorShape::SizeHorCursor);
    lineEdit()->installEventFilter(this);
}

bool DraggableDoubleSpinBox::eventFilter(QObject *obj, QEvent *event) {
    if (obj == lineEdit()) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (!isHolding) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                holdStartPos = mouseEvent->position();
                holdStartValue = value();
                isDragging = false;
                isHolding = true;
                holdCompleteStartPos = QCursor::pos();
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (isHolding) {
                float moved = mouseEvent->position().x() - holdStartPos.x();
                if (isDragging) {
                    setValue(holdStartValue + moved);
                    if ((QCursor::pos() - holdAfterDragStartPos)
                            .manhattanLength() > 100) {
                        holdStartValue = value();
                        QCursor::setPos(holdAfterDragStartPos);
                    }
                    return true;
                } else {
                    if (qAbs(moved) >= QApplication::startDragDistance()) {
                        holdAfterDragStartPos = QCursor::pos();
                        holdStartPos = mouseEvent->position();
                        isDragging = true;
                        QApplication::setOverrideCursor(
                            Qt::CursorShape::BlankCursor);
                    }
                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease ||
                   event->type() == QEvent::ContextMenu) {
            if (isDragging) {
                isDragging = false;
                QCursor::setPos(holdCompleteStartPos);
                QApplication::restoreOverrideCursor();
                clearFocus();
            }
            isHolding = false;
        }
    }
    return QObject::eventFilter(obj, event);
}
