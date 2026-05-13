#include "TextArea.h"

#include "Layout.h"
#include "Window.h"

#include <algorithm>

namespace gui {

    TextArea::TextArea()
        : Edit() {
        SetLocalBounds(Rectangle(0, 0, 200, 120));
    }

    void TextArea::OnCreate() {
        Edit::OnCreate();
    }

    void TextArea::InsertChar(char c) {
        // Newlines are always accepted in a multiline editor; other chars go through the filter.
        if (c != '\n' && !AcceptChar(c))
            return;
        if (m_controller.caretIndex < 0)
            m_controller.caretIndex = 0;
        if (m_controller.caretIndex > (int)text().size())
            m_controller.caretIndex = (int)text().size();

        std::string s = text();
        s.insert(m_controller.caretIndex, 1, c);
        m_controller.caretIndex++;
        m_controller.internalEdit = true;
        text = s;
        m_controller.internalEdit = false;
    }

    void TextArea::RemoveChar(int i) {
        if (i < 0 || i >= (int)text().size())
            return;
        std::string s = text();
        s.erase(i, 1);
        m_controller.internalEdit = true;
        text = s;
        m_controller.internalEdit = false;
    }

    void TextArea::DeleteSelected() {
        if (!IsSelected())
            return;
        int a = m_controller.selectionStart, b = m_controller.selectionEnd;
        if (a > b)
            std::swap(a, b);
        a = std::max(0, a);
        b = std::min(b, (int)text().size());
        std::string s = text();
        s.erase(a, b - a);
        m_controller.caretIndex = a;
        Deselect();
        m_controller.internalEdit = true;
        text = s;
        m_controller.internalEdit = false;
    }

    void TextArea::Rebuild() {
        if (!m_window)
            return;
        auto textStyle = GetStyle();
        auto& g = GetWindow()->GetGraphics();

        m_lines = text::ComputeLines(g, textStyle, text());

        Color defaultColor = Color::FromStyle(textStyle["color"]);
        for (auto& line : m_lines) {
            for (auto& ch : line.chars) {
                ch.color = defaultColor;
                ch.style = FontStyle::Normal;
            }
        }

        if (!m_lines.empty()) {
            text::ApplyTextStyle(g, textStyle);
            m_lineHeight = static_cast<int>(g.GetFontExtents().height);
            int maxW = 0;
            for (const auto& line : m_lines)
                maxW = std::max(maxW, line.bounds.w);
            m_controller.textSize = {maxW, m_lineHeight * (int)m_lines.size()};
        } else {
            m_controller.textSize = {0, 0};
        }
    }

    // -------------------------------------------------------------------------
    // ComputeLineStarts
    // Returns the global text() index at which each line begins.
    // -------------------------------------------------------------------------
    std::vector<int> TextArea::ComputeLineStarts() const {
        std::vector<int> starts;
        int offset = 0;
        for (const auto& line : m_lines) {
            starts.push_back(offset);
            // chars.size() == visible chars + sentinel; sentinel absorbs the '\n'
            offset += (int)line.chars.size();
        }
        return starts;
    }

    int TextArea::HitTestIndex(int localX, int localY) const {
        if (m_lines.empty() || m_lineHeight <= 0)
            return 0;
        EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
        int tx = localX - (int)pad.left;
        int ty = localY - (int)pad.top;
        int lineIdx = ty / m_lineHeight;
        lineIdx = std::max(0, std::min(lineIdx, (int)m_lines.size() - 1));
        auto starts = ComputeLineStarts();
        const auto& line = m_lines[lineIdx];
        int lineStart = starts[lineIdx];
        for (int i = 0; i < (int)line.chars.size() - 1; ++i) {
            const auto& c = line.chars[i];
            if (tx >= c.bounds.x && tx < c.bounds.x + c.bounds.w)
                return lineStart + c.index;
        }
        return lineStart + (int)line.chars.size() - 1;
    }

    TextArea::LineInfo TextArea::GetCaretLineInfo() const {
        auto starts = ComputeLineStarts();
        int lineIdx = 0;
        for (int i = (int)starts.size() - 1; i >= 0; --i) {
            if (m_controller.caretIndex >= starts[i]) {
                lineIdx = i;
                break;
            }
        }
        int lineStart = starts[lineIdx];
        int lineLen = (int)m_lines[lineIdx].chars.size() - 1; // exclude sentinel
        return {lineIdx, lineStart, lineLen};
    }

    void TextArea::OnMouseDown(MouseEvent e) {
        if (!m_controller.editable || e.button != MouseButton::Left)
            return;
        m_controller.caretIndex = HitTestIndex(e.x, e.y);
        m_controller.state = text::EditState::Selecting;
        m_controller.selectionStart = m_controller.caretIndex;
        m_controller.selectionEnd = -1;
        Invalidate();
    }

    void TextArea::OnMouseMove(MotionEvent e) {
        if (m_controller.state != text::EditState::Selecting)
            return;
        int idx = HitTestIndex(e.x, e.y);
        m_controller.selectionEnd = idx;
        m_controller.caretIndex = idx;
        Invalidate();
    }

    void TextArea::OnDraw(Graphics& g) {
        if (!m_window)
            return;

        // Draw styled background (same logic as Edit::OnDraw)
        auto sz = GetSize();
        std::string state = "normal";
        if (IsFocused())
            state = "focused";
        else if (m_hovered)
            state = "hover";
        g.StyledRect(0, 0, sz.w, sz.h, GetStyle()[state]);

        auto style = GetStyle();
        text::ApplyTextStyle(g, style);
        auto fm = g.GetFontExtents();
        int lineHeight = static_cast<int>(fm.height);

        EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
        int visW = sz.w - pad.left - pad.right;
        int visH = sz.h - pad.top - pad.bottom;

        g.ClipPushRect(pad.left, pad.top, visW, visH);
        g.Translate(pad.left, pad.top);

        // --- Selection background (multi-line aware) ---
        if (IsSelected()) {
            int selA = m_controller.selectionStart, selB = m_controller.selectionEnd;
            if (selA > selB)
                std::swap(selA, selB);

            auto starts = ComputeLineStarts();
            std::vector<text::Line> selLines;

            for (int li = 0; li < (int)m_lines.size(); ++li) {
                const auto& line = m_lines[li];
                int lineStart = starts[li];

                if (selB <= lineStart || selA >= lineStart + (int)line.chars.size())
                    continue;

                int localA = std::max(0, selA - lineStart);
                int localB = std::min((int)line.chars.size() - 1, selB - lineStart);
                localA = std::min(localA, (int)line.chars.size() - 1);
                localB = std::min(localB, (int)line.chars.size() - 1);

                int x0 = line.chars[localA].bounds.x;
                int x1 = line.chars[localB].bounds.x;
                if (x1 <= x0 && localB > localA)
                    x1 = x0 + 4;

                text::Line selLine;
                selLine.bounds = {x0, li * lineHeight, x1 - x0, lineHeight};
                selLines.push_back(selLine);
            }

            if (!selLines.empty()) {
                auto hull = text::BuildOrthoHull(selLines);
                g.BeginSimplePath();
                for (const auto& pt : hull)
                    g.AddPathPoint(pt.x, pt.y);
                g.EndSimplePath(true);
                g.StyledPaint(GetWindow()->GetApp()->GetStyle()["Selection"]);
            }
        }

        // --- Text ---
        const std::string fontName = style.value("font", "Sans");
        const double fontSize = style.value("fontSize", 14.0);
        for (int li = 0; li < (int)m_lines.size(); ++li) {
            int lineY = li * lineHeight;
            const auto& line = m_lines[li];
            for (const text::Char& chr : line.chars) {
                if (chr.value == '\0')
                    continue;
                g.Color(chr.color.r, chr.color.g, chr.color.b, chr.color.a);
                g.Font(chr.style, fontName, fontSize);
                g.DrawChar(chr.value, chr.bounds.x, lineY + lineHeight);
            }
        }

        // --- Caret ---
        if (IsFocused() && m_controller.editable && !m_lines.empty() && m_controller.showCaret) {
            auto info = GetCaretLineInfo();
            const auto& line = m_lines[info.lineIdx];
            int localCaretIdx = m_controller.caretIndex - info.lineStart;
            localCaretIdx = std::max(0, std::min(localCaretIdx, (int)line.chars.size() - 1));
            int caretX = line.chars[localCaretIdx].bounds.x;
            int caretY = info.lineIdx * lineHeight;

            Color caretColor = Color::FromStyle(style["color"]);
            g.Color(caretColor.r, caretColor.g, caretColor.b, 1.0f);
            g.LineWidth(1.0f);
            g.Line(caretX, caretY, caretX, caretY + lineHeight + 1);
            g.Stroke();
        }

        g.ClipPop();
    }

    void TextArea::OnKeyDown(KeyEvent e) {
        if (!m_controller.editable)
            return;

        auto beginSel = [&]() {
            if (e.mod.shift && m_controller.selectionStart == -1)
                m_controller.selectionStart = m_controller.caretIndex;
        };
        auto endSel = [&]() {
            if (e.mod.shift)
                m_controller.selectionEnd = m_controller.caretIndex;
            else
                Deselect();
        };

        if (e.key == Key::Enter) {
            if (IsSelected())
                DeleteSelected();
            InsertChar('\n');
            Invalidate();
            return;
        }

        if (e.key == Key::Up) {
            beginSel();
            auto info = GetCaretLineInfo();
            if (info.lineIdx > 0) {
                auto starts = ComputeLineStarts();
                int localCol = m_controller.caretIndex - info.lineStart;
                int prevLineStart = starts[info.lineIdx - 1];
                int prevLineLen = (int)m_lines[info.lineIdx - 1].chars.size() - 1;
                m_controller.caretIndex = prevLineStart + std::min(localCol, prevLineLen);
            }
            endSel();
            Invalidate();
            return;
        }

        if (e.key == Key::Down) {
            beginSel();
            auto info = GetCaretLineInfo();
            if (info.lineIdx < (int)m_lines.size() - 1) {
                auto starts = ComputeLineStarts();
                int localCol = m_controller.caretIndex - info.lineStart;
                int nextLineStart = starts[info.lineIdx + 1];
                int nextLineLen = (int)m_lines[info.lineIdx + 1].chars.size() - 1;
                m_controller.caretIndex = nextLineStart + std::min(localCol, nextLineLen);
            }
            endSel();
            Invalidate();
            return;
        }

        if (e.key == Key::Home) {
            beginSel();
            auto info = GetCaretLineInfo();
            m_controller.caretIndex = info.lineStart;
            endSel();
            Invalidate();
            return;
        }

        if (e.key == Key::End) {
            beginSel();
            auto info = GetCaretLineInfo();
            m_controller.caretIndex = info.lineStart + info.lineLen;
            endSel();
            Invalidate();
            return;
        }

        m_controller.HandleKeyDown(e);
    }

    Size TextArea::GetPreferredSize() const {
        if (!IsAutoSize())
            return Edit::GetPreferredSize();
        Size sz = GetSize();
        if (m_controller.textSize.w > 0 || m_controller.textSize.h > 0) {
            EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
            return {
                m_controller.textSize.w + (int)pad.left + (int)pad.right,
                m_controller.textSize.h + (int)pad.top + (int)pad.bottom
            };
        }
        return sz;
    }

    void TextArea::Format(int from, int len, FontStyle style, float r, float g, float b) {
        int textSize = (int)text().size();
        from = std::max(0, std::min(from, textSize));
        int to = std::min(from + len, textSize);
        Color color = Color::FromRGB(r, g, b);
        int lineOffset = 0;
        for (auto& line : m_lines) {
            for (auto& ch : line.chars) {
                if (ch.value != '\0') {
                    int gi = lineOffset + ch.index;
                    if (gi >= from && gi < to) {
                        ch.color = color;
                        ch.style = style;
                    }
                }
            }
            lineOffset += (int)line.chars.size();
        }
        Invalidate();
    }

    void TextArea::Format(FontStyle style, float r, float g, float b) {
        if (!IsSelected())
            return;
        int selA = m_controller.selectionStart, selB = m_controller.selectionEnd;
        if (selA > selB)
            std::swap(selA, selB);
        Format(selA, selB - selA, style, r, g, b);
    }

} // namespace gui
