#include "ShortcutEdit.h"
#include <QKeyEvent>
#include <QFocusEvent>
#include <QApplication>

ShortcutEdit::ShortcutEdit(QWidget* parent)
    : QLineEdit(parent)
{
    setPlaceholderText(tr("Press keys..."));
    setReadOnly(true);
    setFocusPolicy(Qt::StrongFocus);
}

QKeySequence ShortcutEdit::keySequence() const
{
    return m_keySequence;
}

void ShortcutEdit::setKeySequence(const QKeySequence& sequence)
{
    if (m_keySequence != sequence)
    {
        m_keySequence = sequence;
        updateDisplay();
        emit keySequenceChanged(sequence);
    }
}

void ShortcutEdit::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    
    // Ignore modifier keys alone
    if (key == Qt::Key_Control || key == Qt::Key_Shift || 
        key == Qt::Key_Alt || key == Qt::Key_Meta)
    {
        return;
    }
    
    // Get modifiers
    Qt::KeyboardModifiers modifiers = event->modifiers();
    
    // Ignore if no modifiers (except for special keys like Delete)
    if (modifiers == Qt::NoModifier && 
        key != Qt::Key_Delete && key != Qt::Key_Backspace &&
        key != Qt::Key_Return && key != Qt::Key_Enter)
    {
        // Allow Escape to clear
        if (key == Qt::Key_Escape)
        {
            setKeySequence(QKeySequence());
            return;
        }
        // For other keys without modifiers, ignore
        return;
    }
    
    // Build key sequence
    int keyCode = key;
    if (modifiers & Qt::ControlModifier) keyCode += Qt::CTRL;
    if (modifiers & Qt::ShiftModifier) keyCode += Qt::SHIFT;
    if (modifiers & Qt::AltModifier) keyCode += Qt::ALT;
    if (modifiers & Qt::MetaModifier) keyCode += Qt::META;
    
    QKeySequence newSequence(keyCode);
    setKeySequence(newSequence);
    
    event->accept();
}

void ShortcutEdit::focusInEvent(QFocusEvent* event)
{
    QLineEdit::focusInEvent(event);
    setStyleSheet("border: 2px solid #3b82f6;");
}

void ShortcutEdit::focusOutEvent(QFocusEvent* event)
{
    QLineEdit::focusOutEvent(event);
    setStyleSheet("");
}

void ShortcutEdit::updateDisplay()
{
    if (m_keySequence.isEmpty())
    {
        setText("");
        setPlaceholderText(tr("Press keys..."));
    }
    else
    {
        setText(m_keySequence.toString(QKeySequence::NativeText));
    }
}

