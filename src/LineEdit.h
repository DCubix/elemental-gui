#pragma once

#include "Element.h"
#include "TextProcessing.h"

namespace gui
{
    class LineEdit : public Element {
    public:
        LineEdit();

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "DefaultText"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseMove(MotionEvent e) override;
        void OnKeyDown(KeyEvent e) override;
        void OnTextInput(TextInputEvent e) override;
        void OnFocus(FocusEvent e) override;
        void OnBlur(BlurEvent e) override;

        Size GetPreferredSize() const override;

        std::string GetText() const { return m_textRaw; }
        void SetText(const std::string& txt);

        bool IsMasked() const { return m_masked; }
        void SetMasked(bool m) { m_masked = m; Rebuild(); Invalidate(); }

        bool IsEditable() const { return m_editable; }
        void SetEditable(bool m) { m_editable = m; }

        void Select(int from, int len = -1);
        void Deselect();
        bool IsSelected() const;

        void SetOnChange(const ValueChanged<std::string>& cb) { m_onChange = cb; }

    protected:
        std::string m_textRaw;
        text::Line m_text;

        bool m_masked{ false }, m_editable{ true };
        int m_caretIndex{ 0 };
        int m_selectionStart{ -1 }, m_selectionEnd{ -1 };
        int m_offsetX{ 0 };
        Size m_textSize{ 0, 0 };

        text::EditState m_state{ text::EditState::Normal };
        ValueChanged<std::string> m_onChange;

        virtual void InsertChar(char c);
        virtual void RemoveChar(int i);
        virtual void DeleteSelected();
        virtual void Rebuild();
    };
}
