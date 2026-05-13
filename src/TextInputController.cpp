#include "TextInputController.h"

#include "Element.h"
#include "Window.h"

#include <algorithm>

namespace gui {

    void TextInputController::Init(Element* owner, Property<std::string>* text) {
        m_owner = owner;
        m_text = text;

        // Default ops call the controller's own methods directly.
        // LineEdit overrides these with virtual-dispatch wrappers so TextArea works.
        m_ops = {
            [this](char c) { InsertChar(c); },
            [this](int i) { RemoveChar(i); },
            [this]() { DeleteSelected(); },
            []() {}
        };

        blinkTimer.Start(500, [this]() {
            showCaret = !showCaret;
            if (m_owner && m_owner->IsFocused())
                m_owner->Invalidate();
        });
    }

    void TextInputController::Rebuild(Graphics& g, const Json& style) {
        const std::string display = masked ? std::string((*m_text)().size(), '*') : (*m_text)();
        auto lines = text::ComputeLines(g, style, display);
        textLine = lines.empty() ? text::Line{} : lines[0];

        Color defaultColor = Color::FromStyle(style["color"]);
        for (auto& ch : textLine.chars)
            ch.color = defaultColor;

        textSize = text::ComputeTextSize(g, style, display);
    }

    void TextInputController::Draw(
        Graphics& g, const Json& style, const Rectangle& bounds, bool focused) {
        text::ApplyTextStyle(g, style);

        auto fm = g.GetFontExtents();
        int lineHeight = static_cast<int>(fm.ascent + fm.descent);

        EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
        int visW = bounds.w - (int)pad.left - (int)pad.right;
        int ty = (bounds.h - lineHeight) / 2;

        if (!textLine.chars.empty()) {
            int caretX =
                textLine.chars[std::min(caretIndex, (int)textLine.chars.size() - 1)].bounds.x;
            int pos = offsetX + caretX;
            if (pos >= visW)
                offsetX = visW - caretX - 1;
            else if (pos < 0)
                offsetX = -caretX;
            if (offsetX > 0)
                offsetX = 0;
        }

        g.ClipPushRect(bounds.x + (int)pad.left, bounds.y, visW, bounds.h);
        g.Translate(bounds.x + (int)pad.left + offsetX, bounds.y + ty);

        if (IsSelected()) {
            int a = selectionStart, b = selectionEnd;
            if (a > b)
                std::swap(a, b);
            a = std::max(0, std::min(a, (int)textLine.chars.size() - 1));
            b = std::max(0, std::min(b, (int)textLine.chars.size() - 1));
            int x0 = textLine.chars[a].bounds.x;
            int x1 = textLine.chars[b].bounds.x;
            g.BeginSimplePath();
            g.AddPathRect(x0, 0, x1 - x0, lineHeight);
            g.EndSimplePath(true);
            g.StyledPaint(m_owner->GetWindow()->GetApp()->GetStyle()["Selection"]);
        }

        const std::string fontName = style.value("font", "Sans");
        const double fontSize = style.value("fontSize", 14.0);
        for (const text::Char& chr : textLine.chars) {
            if (chr.value == '\0')
                continue;
            g.Color(chr.color.r, chr.color.g, chr.color.b, chr.color.a);
            g.Font(chr.style, fontName, fontSize);
            g.DrawChar(masked ? '*' : chr.value, chr.bounds.x, chr.bounds.y + fm.ascent);
        }

        if (focused && editable && showCaret) {
            int caretX = textLine.chars.empty()
                ? 0
                : textLine.chars[std::min(caretIndex, (int)textLine.chars.size() - 1)].bounds.x;
            Color caretColor = Color::FromStyle(style["color"]);
            g.Color(caretColor.r, caretColor.g, caretColor.b, 1.0f);
            g.LineWidth(1.0f);
            g.Line(caretX, 0, caretX, lineHeight + 1);
            g.Stroke();
        }

        g.ClipPop();
    }

    // ---- Event handlers ---------------------------------------------------------

    void TextInputController::HandleMouseDown(
        MouseEvent e, const EdgeInsets& pad, int originX) {
        if (!editable || e.button != MouseButton::Left)
            return;
        int effectiveX = e.x - originX - (int)pad.left - offsetX;
        for (const text::Char& c : textLine.chars) {
            if (effectiveX >= c.bounds.x && effectiveX < c.bounds.x + c.bounds.w) {
                caretIndex = c.index;
                break;
            }
        }
        state = text::EditState::Selecting;
        selectionStart = caretIndex;
        selectionEnd = -1;
        if (m_owner)
            m_owner->Invalidate();
    }

    void TextInputController::HandleMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (state == text::EditState::Selecting) {
            state = text::EditState::Normal;
            if (m_owner)
                m_owner->Invalidate();
        }
    }

    void TextInputController::HandleMouseMove(
        MotionEvent e, const EdgeInsets& pad, int originX) {
        if (state != text::EditState::Selecting)
            return;
        int effectiveX = e.x - originX - (int)pad.left - offsetX;
        for (const text::Char& c : textLine.chars) {
            if (effectiveX >= c.bounds.x && effectiveX < c.bounds.x + c.bounds.w) {
                selectionEnd = c.index;
                caretIndex = c.index;
                break;
            }
        }
        if (m_owner)
            m_owner->Invalidate();
    }

    void TextInputController::HandleKeyDown(KeyEvent e) {
        if (!editable)
            return;

        auto beginSel = [&]() {
            if (e.mod.shift && selectionStart == -1)
                selectionStart = caretIndex;
        };
        auto endSel = [&]() {
            if (e.mod.shift)
                selectionEnd = caretIndex;
            else
                Deselect();
        };

        if (e.key == Key::Home) {
            beginSel();
            caretIndex = 0;
            endSel();
        } else if (e.key == Key::End) {
            beginSel();
            caretIndex = (int)(*m_text)().size();
            endSel();
        } else if (e.key == Key::Backspace) {
            if (IsSelected()) {
                m_ops.deleteSelected();
                m_ops.notify();
            } else if (caretIndex > 0) {
                caretIndex--;
                m_ops.removeChar(caretIndex);
                m_ops.notify();
            }
        } else if (e.key == Key::Delete) {
            if (IsSelected()) {
                m_ops.deleteSelected();
                m_ops.notify();
            } else if (caretIndex < (int)(*m_text)().size()) {
                m_ops.removeChar(caretIndex);
                m_ops.notify();
            }
        } else if (e.key == Key::Left) {
            beginSel();
            if (caretIndex > 0)
                caretIndex--;
            endSel();
        } else if (e.key == Key::Right) {
            beginSel();
            if (caretIndex < (int)(*m_text)().size())
                caretIndex++;
            endSel();
        } else if (e.key == Key::C && e.mod.control && IsSelected()) {
            int a = selectionStart, b = selectionEnd;
            if (a > b)
                std::swap(a, b);
            if (m_owner && m_owner->GetWindow())
                m_owner->GetWindow()->GetApp()->SetClipboard((*m_text)().substr(a, b - a));
        } else if (e.key == Key::X && e.mod.control && IsSelected()) {
            int a = selectionStart, b = selectionEnd;
            if (a > b)
                std::swap(a, b);
            if (m_owner && m_owner->GetWindow())
                m_owner->GetWindow()->GetApp()->SetClipboard((*m_text)().substr(a, b - a));
            m_ops.deleteSelected();
            m_ops.notify();
        } else if (e.key == Key::V && e.mod.control) {
            if (IsSelected())
                m_ops.deleteSelected();
            if (m_owner && m_owner->GetWindow()) {
                for (char c : m_owner->GetWindow()->GetApp()->GetClipboard())
                    m_ops.insertChar(c);
            }
            m_ops.notify();
        } else if (e.key == Key::A && e.mod.control) {
            Select(0);
        }

        if (m_owner)
            m_owner->Invalidate();
    }

    void TextInputController::HandleTextInput(TextInputEvent e) {
        if (!editable)
            return;
        if (IsSelected())
            m_ops.deleteSelected();
        m_ops.insertChar(e.inputChar);
        m_ops.notify();
        if (m_owner)
            m_owner->Invalidate();
    }

    // ---- Editing operations -----------------------------------------------------

    bool TextInputController::AcceptChar(char c) const {
        return m_filter.Test(c, (*m_text)(), caretIndex);
    }

    void TextInputController::InsertChar(char c) {
        if (c == '\r' || c == '\n')
            return;
        if (!AcceptChar(c))
            return;
        if (caretIndex < 0)
            caretIndex = 0;
        if (caretIndex > (int)(*m_text)().size())
            caretIndex = (int)(*m_text)().size();
        std::string s = (*m_text)();
        s.insert(caretIndex, 1, c);
        caretIndex++;
        internalEdit = true;
        *m_text = s;
        internalEdit = false;
    }

    void TextInputController::RemoveChar(int i) {
        if (i < 0 || i >= (int)(*m_text)().size())
            return;
        std::string s = (*m_text)();
        s.erase(i, 1);
        internalEdit = true;
        *m_text = s;
        internalEdit = false;
    }

    void TextInputController::DeleteSelected() {
        if (!IsSelected())
            return;
        int a = selectionStart, b = selectionEnd;
        if (a > b)
            std::swap(a, b);
        a = std::max(0, a);
        b = std::min(b, (int)(*m_text)().size());
        std::string s = (*m_text)();
        s.erase(a, b - a);
        caretIndex = a;
        Deselect();
        internalEdit = true;
        *m_text = s;
        internalEdit = false;
    }

    void TextInputController::Select(int from, int len) {
        if (len < 0)
            len = (int)(*m_text)().size() - from;
        selectionStart = from;
        selectionEnd = from + len;
        if (m_owner)
            m_owner->Invalidate();
    }

    void TextInputController::Deselect() {
        selectionStart = -1;
        selectionEnd = -1;
        if (m_owner)
            m_owner->Invalidate();
    }

    bool TextInputController::IsSelected() const {
        return selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd;
    }

} // namespace gui
