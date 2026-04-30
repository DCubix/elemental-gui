#include "LineEdit.h"

#include "Layout.h"
#include "Window.h"

#include <algorithm>

namespace gui {
    LineEdit::LineEdit()
        : Element() {
        SetLocalBounds(Rectangle(0, 0, 120, 28));

        m_blinkTimer.Start(500, [this]() {
            m_showCaret = !m_showCaret;
            if (IsFocused())
                Invalidate();
        });
    }

    void LineEdit::OnDraw(Graphics& g) {
        if (!m_window)
            return;

        auto style = GetStyle();
        text::ApplyTextStyle(g, style);

        auto fm = g.GetFontExtents();
        int lineHeight = static_cast<int>(fm.ascent + fm.descent);

        Size sz = GetSize();
        EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);

        int visW = sz.w - pad.left - pad.right;
        int ty = (sz.h - lineHeight) / 2; // vertical center

        // Update horizontal scroll offset so caret stays visible
        if (!m_text.chars.empty()) {
            int caretX =
                m_text.chars[std::min(m_caretIndex, (int)m_text.chars.size() - 1)].bounds.x;
            int pos = m_offsetX + caretX;
            if (pos >= visW)
                m_offsetX = visW - caretX - 1;
            else if (pos < 0)
                m_offsetX = -caretX;
            if (m_offsetX > 0)
                m_offsetX = 0;
        }

        // Clip to content area (no vertical padding — text is vertically centered)
        g.ClipPushRect(pad.left, 0, visW, sz.h);
        g.Translate(pad.left + m_offsetX, ty);

        // Selection background
        if (IsSelected()) {
            int a = m_selectionStart, b = m_selectionEnd;
            if (a > b)
                std::swap(a, b);
            a = std::max(0, std::min(a, (int)m_text.chars.size() - 1));
            b = std::max(0, std::min(b, (int)m_text.chars.size() - 1));
            int x0 = m_text.chars[a].bounds.x;
            int x1 = m_text.chars[b].bounds.x;
            g.BeginSimplePath();
            g.AddPathRect(x0, 0, x1 - x0, lineHeight);
            g.EndSimplePath(true);
            g.StyledPaint(GetWindow()->GetApp()->GetStyle()["Selection"]);
        }

        // Text
        const std::string fontName = style.value("font", "Sans");
        const double fontSize = style.value("fontSize", 14.0);
        for (const text::Char& chr : m_text.chars) {
            if (chr.value == '\0')
                continue;
            g.Color(chr.color.r, chr.color.g, chr.color.b, chr.color.a);
            g.Font(chr.style, fontName, fontSize);
            g.DrawChar(m_masked ? '*' : chr.value, chr.bounds.x, chr.bounds.y + fm.ascent);
        }

        // Caret
        if (IsFocused() && m_editable && m_showCaret) {
            int caretX =
                m_text.chars.empty()
                    ? 0
                    : m_text.chars[std::min(m_caretIndex, (int)m_text.chars.size() - 1)].bounds.x;
            Color caretColor = Color::FromStyle(style["color"]);
            g.Color(caretColor.r, caretColor.g, caretColor.b, 1.0f);
            g.LineWidth(1.0f);
            g.Line(caretX, 0, caretX, lineHeight + 1);
            g.Stroke();
        }

        // Undo translate (ClipPop restores the full cairo state including transform)
        g.ClipPop();
    }

    void LineEdit::OnMouseDown(MouseEvent e) {
        if (!m_editable)
            return;
        if (e.button != MouseButton::Left)
            return;

        EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
        int effectiveX = e.x - pad.left - m_offsetX;
        for (const text::Char& c : m_text.chars) {
            if (effectiveX >= c.bounds.x && effectiveX < c.bounds.x + c.bounds.w) {
                m_caretIndex = c.index;
                break;
            }
        }
        m_state = text::EditState::Selecting;
        m_selectionStart = m_caretIndex;
        m_selectionEnd = -1;
        Invalidate();
    }

    void LineEdit::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (m_state == text::EditState::Selecting) {
            m_state = text::EditState::Normal;
            Invalidate();
        }
    }

    void LineEdit::OnMouseMove(MotionEvent e) {
        if (m_state != text::EditState::Selecting)
            return;
        EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
        int effectiveX = e.x - pad.left - m_offsetX;
        for (const text::Char& c : m_text.chars) {
            if (effectiveX >= c.bounds.x && effectiveX < c.bounds.x + c.bounds.w) {
                m_selectionEnd = c.index;
                m_caretIndex = c.index;
                break;
            }
        }
        Invalidate();
    }

    void LineEdit::OnKeyDown(KeyEvent e) {
        if (!m_editable)
            return;

        auto beginSel = [&]() {
            if (e.mod.shift && m_selectionStart == -1)
                m_selectionStart = m_caretIndex;
        };
        auto endSel = [&]() {
            if (e.mod.shift)
                m_selectionEnd = m_caretIndex;
            else
                Deselect();
        };

        if (e.key == Key::Home) {
            beginSel();
            m_caretIndex = 0;
            endSel();
        } else if (e.key == Key::End) {
            beginSel();
            m_caretIndex = (int)m_textRaw.size();
            endSel();
        } else if (e.key == Key::Backspace) {
            if (IsSelected())
                DeleteSelected();
            else if (m_caretIndex > 0) {
                m_caretIndex--;
                RemoveChar(m_caretIndex);
            }
        } else if (e.key == Key::Delete) {
            if (IsSelected())
                DeleteSelected();
            else if (m_caretIndex < (int)m_textRaw.size()) {
                RemoveChar(m_caretIndex);
            }
        } else if (e.key == Key::Left) {
            beginSel();
            if (m_caretIndex > 0)
                m_caretIndex--;
            endSel();
        } else if (e.key == Key::Right) {
            beginSel();
            if (m_caretIndex < (int)m_textRaw.size())
                m_caretIndex++;
            endSel();
        } else if (e.key == Key::C && e.mod.control && IsSelected()) {
            int a = m_selectionStart, b = m_selectionEnd;
            if (a > b)
                std::swap(a, b);
            m_window->GetApp()->SetClipboard(m_textRaw.substr(a, b - a));
        } else if (e.key == Key::X && e.mod.control && IsSelected()) {
            int a = m_selectionStart, b = m_selectionEnd;
            if (a > b)
                std::swap(a, b);
            m_window->GetApp()->SetClipboard(m_textRaw.substr(a, b - a));
            DeleteSelected();
        } else if (e.key == Key::V && e.mod.control) {
            if (IsSelected())
                DeleteSelected();
            for (char c : m_window->GetApp()->GetClipboard())
                InsertChar(c);
        } else if (e.key == Key::A && e.mod.control) {
            Select(0);
        }
        Invalidate();
    }

    void LineEdit::OnTextInput(TextInputEvent e) {
        if (!m_editable)
            return;
        if (IsSelected())
            DeleteSelected();
        InsertChar(e.inputChar);
        Invalidate();
    }

    void LineEdit::OnFocus(FocusEvent e) {
        Invalidate();
    }

    void LineEdit::OnBlur(BlurEvent e) {
        Invalidate();
    }

    Size LineEdit::GetPreferredSize() const {
        if (!IsAutoSize()) {
            return Element::GetPreferredSize();
        }
        return m_textSize.w > 0 ? m_textSize : Size{120, 28};
    }

    void LineEdit::SetText(const std::string& txt) {
        if (m_textRaw == txt)
            return;
        m_textRaw = txt;
        m_caretIndex = (int)m_textRaw.size();
        m_selectionStart = -1;
        m_selectionEnd = -1;
        m_offsetX = 0;
        Rebuild();
        Invalidate();
        if (m_onChange)
            m_onChange(m_textRaw);
    }

    void LineEdit::Select(int from, int len) {
        if (len < 0)
            len = (int)m_textRaw.size() - from;
        m_selectionStart = from;
        m_selectionEnd = from + len;
        Invalidate();
    }

    void LineEdit::Deselect() {
        m_selectionStart = -1;
        m_selectionEnd = -1;
        Invalidate();
    }

    bool LineEdit::IsSelected() const {
        return m_selectionStart != -1 && m_selectionEnd != -1 && m_selectionStart != m_selectionEnd;
    }

    void LineEdit::InsertChar(char c) {
        if (c == '\r' || c == '\n')
            return; // single-line blocks newlines
        if (m_caretIndex < 0)
            m_caretIndex = 0;
        if (m_caretIndex > (int)m_textRaw.size())
            m_caretIndex = (int)m_textRaw.size();
        m_textRaw.insert(m_caretIndex, 1, c);
        m_caretIndex++;
        Rebuild();
        if (m_onChange)
            m_onChange(m_textRaw);
    }

    void LineEdit::RemoveChar(int i) {
        if (i < 0 || i >= (int)m_textRaw.size())
            return;
        m_textRaw.erase(i, 1);
        Rebuild();
        if (m_onChange)
            m_onChange(m_textRaw);
    }

    void LineEdit::DeleteSelected() {
        if (!IsSelected())
            return;
        int a = m_selectionStart, b = m_selectionEnd;
        if (a > b)
            std::swap(a, b);
        a = std::max(0, a);
        b = std::min(b, (int)m_textRaw.size());
        m_textRaw.erase(a, b - a);
        m_caretIndex = a;
        Deselect();
        Rebuild();
        if (m_onChange)
            m_onChange(m_textRaw);
    }

    void LineEdit::Rebuild() {
        if (!m_window)
            return;
        auto textStyle = GetStyle();
        const std::string display = m_masked ? std::string(m_textRaw.size(), '*') : m_textRaw;
        auto& g = GetWindow()->GetGraphics();

        auto lines = text::ComputeLines(g, textStyle, display);
        m_text = lines.empty() ? text::Line{} : lines[0];

        // Apply default text color to all chars
        Color defaultColor = Color::FromStyle(textStyle["color"]);
        for (auto& ch : m_text.chars)
            ch.color = defaultColor;

        m_textSize = text::ComputeTextSize(g, textStyle, display);
    }
} // namespace gui
