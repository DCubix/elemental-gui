#pragma once

#include "Element.h"
#include "InputFilter.h"
#include "TextInputController.h"

namespace gui {
    class LineEdit : public Element {
    public:
        LineEdit();

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "DefaultText"; }
        bool NeedsTextInput() const override { return true; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseMove(MotionEvent e) override;
        void OnKeyDown(KeyEvent e) override;
        void OnTextInput(TextInputEvent e) override;
        void OnFocus(FocusEvent e) override;
        void OnBlur(BlurEvent e) override;

        Size GetPreferredSize() const override;

        bool IsMasked() const { return m_controller.masked; }
        void SetMasked(bool m) {
            m_controller.masked = m;
            Rebuild();
            Invalidate();
        }

        bool IsEditable() const { return m_controller.editable; }
        void SetEditable(bool e) { m_controller.editable = e; }

        void Select(int from, int len = -1);
        void Deselect();
        bool IsSelected() const;

        void SetInputFilter(const InputFilter& filter) { m_controller.SetFilter(filter); }
        const InputFilter& GetInputFilter() const { return m_controller.GetFilter(); }

        Property<std::string> text;

    protected:
        TextInputController m_controller;

        // Returns true if c passes the active InputFilter at the current caret position.
        // Override-friendly: call this in subclass InsertChar implementations.
        bool AcceptChar(char c) const { return m_controller.AcceptChar(c); }

        virtual void InsertChar(char c);
        virtual void RemoveChar(int i);
        virtual void DeleteSelected();
        virtual void Rebuild();
    };
} // namespace gui
