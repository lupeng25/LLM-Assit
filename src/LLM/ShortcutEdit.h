#pragma once

#include <QLineEdit>
#include <QKeySequence>
#include <QKeyEvent>

class ShortcutEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit ShortcutEdit(QWidget* parent = nullptr);
    
    QKeySequence keySequence() const;
    void setKeySequence(const QKeySequence& sequence);
    
signals:
    void keySequenceChanged(const QKeySequence& sequence);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    QKeySequence m_keySequence;
    void updateDisplay();
};

