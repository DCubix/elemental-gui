#include "LineEdit.h"

#include "Layout.h"
#include "Window.h"

namespace gui {
    LineEdit::LineEdit()
        : Element() {
        SetLocalBounds(Rectangle(0, 0, 120, 28));
        m_controller.Init(this, &text);

        // Wire ops through virtual dispatch so TextArea overrides are honoured.
        m_controller.SetEditOps({
            [this](char c) { InsertChar(c); },
            [this](int i) { RemoveChar(i); },
            [this]() { DeleteSelected(); },
            [this]() { NotifyListeners(); }
        });

        text.Bind([this](const auto&) {
            if (!m_controller.internalEdit) {
                m_controller.caretIndex = (int)text().size();
                m_controller.selectionStart = -1;
                m_controller.selectionEnd = -1;
                m_controller.offsetX = 0;
            }
            Rebuild();    // virtual — TextArea overrides to rebuild m_lines
            Invalidate();
        });
    }

    void LineEdit::OnDraw(Graphics& g) {
        if (!m_window)
            return;
        auto sz = GetSize();
        m_controller.Draw(g, GetStyle(), Rectangle{0, 0, sz.w, sz.h}, IsFocused());
    }

    void LineEdit::OnMouseDown(MouseEvent e) {
        m_controller.HandleMouseDown(e, EdgeInsets::FromStyle(GetStyle()["padding"]));
    }

    void LineEdit::OnMouseUp(MouseEvent e) {
        m_controller.HandleMouseUp(e);
    }

    void LineEdit::OnMouseMove(MotionEvent e) {
        m_controller.HandleMouseMove(e, EdgeInsets::FromStyle(GetStyle()["padding"]));
    }

    void LineEdit::OnKeyDown(KeyEvent e) {
        m_controller.HandleKeyDown(e);
    }

    void LineEdit::OnTextInput(TextInputEvent e) {
        m_controller.HandleTextInput(e);
    }

    void LineEdit::OnFocus(FocusEvent e) {
        Invalidate();
    }

    void LineEdit::OnBlur(BlurEvent e) {
        Invalidate();
    }

    Size LineEdit::GetPreferredSize() const {
        if (!IsAutoSize())
            return Element::GetPreferredSize();
        return m_controller.textSize.w > 0 ? m_controller.textSize : Size{120, 28};
    }

    void LineEdit::Select(int from, int len) {
        m_controller.Select(from, len);
    }

    void LineEdit::Deselect() {
        m_controller.Deselect();
    }

    bool LineEdit::IsSelected() const {
        return m_controller.IsSelected();
    }

    void LineEdit::InsertChar(char c) {
        m_controller.InsertChar(c);
    }

    void LineEdit::RemoveChar(int i) {
        m_controller.RemoveChar(i);
    }

    void LineEdit::DeleteSelected() {
        m_controller.DeleteSelected();
    }

    void LineEdit::Rebuild() {
        if (!m_window)
            return;
        auto style = GetStyle();
        auto& g = GetWindow()->GetGraphics();
        m_controller.Rebuild(g, style);
    }
} // namespace gui
