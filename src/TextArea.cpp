#include "TextArea.h"

#include "Layout.h"
#include "Window.h"

#include <algorithm>

namespace gui {

    TextArea::TextArea() {
        SetLocalBounds(Rectangle(0, 0, 200, 120));
    }

    void TextArea::OnCreate() {
        Edit::OnCreate();
    }

    // -------------------------------------------------------------------------
    // InsertChar
    // -------------------------------------------------------------------------
    void TextArea::InsertChar(char c) {
        if (m_caretIndex < 0)
            m_caretIndex = 0;
        if (m_caretIndex > (int)m_textRaw.size())
            m_caretIndex = (int)m_textRaw.size();

        CharFormat fmt;
        if (m_caretIndex > 0 && m_caretIndex - 1 < (int)m_formats.size())
            fmt = m_formats[m_caretIndex - 1];

        m_textRaw.insert(m_caretIndex, 1, c);
        m_formats.insert(m_formats.begin() + m_caretIndex, fmt);
        m_caretIndex++;
        Rebuild();
        Invalidate();
        if (m_onChange)
            m_onChange(m_textRaw);
    }

    // -------------------------------------------------------------------------
    // RemoveChar
    // -------------------------------------------------------------------------
    void TextArea::RemoveChar(int i) {
        if (i < 0 || i >= (int)m_textRaw.size())
            return;
        m_textRaw.erase(i, 1);
        if (i < (int)m_formats.size())
            m_formats.erase(m_formats.begin() + i);
        Rebuild();
        Invalidate();
        if (m_onChange)
            m_onChange(m_textRaw);
    }

    // -------------------------------------------------------------------------
    // DeleteSelected
    // -------------------------------------------------------------------------
    void TextArea::DeleteSelected() {
        if (!IsSelected())
            return;
        int a = m_selectionStart, b = m_selectionEnd;
        if (a > b)
            std::swap(a, b);
        a = std::max(0, a);
        b = std::min(b, (int)m_textRaw.size());
        m_textRaw.erase(a, b - a);
        if (a < (int)m_formats.size()) {
            int eraseEnd = std::min(b, (int)m_formats.size());
            m_formats.erase(m_formats.begin() + a, m_formats.begin() + eraseEnd);
        }
        m_caretIndex = a;
        Deselect();
        Rebuild();
        Invalidate();
        if (m_onChange)
            m_onChange(m_textRaw);
    }

    // -------------------------------------------------------------------------
    // Rebuild
    // -------------------------------------------------------------------------
    void TextArea::Rebuild() {
        if (!m_window)
            return;
        auto textStyle = GetStyle()["DefaultText"];
        auto& g = GetWindow()->GetGraphics();

        m_lines = text::ComputeLines(g, textStyle, m_textRaw);

        // Apply per-character formats across lines.
        // Each line's chars has (text_length + 1) entries (the +1 is the sentinel '\0').
        // The '\n' separator in m_textRaw occupies one slot in m_formats but is NOT
        // represented as a char in any line — ComputeLines splits on '\n' and the
        // sentinel absorbs the gap. So lineOffset advances by chars.size() per line,
        // which accounts for both the visible chars and the implicit '\n'.
        int lineOffset = 0;
        Color defaultColor = Color::FromStyle(textStyle["color"]);

        for (auto& line : m_lines) {
            for (auto& ch : line.chars) {
                if (ch.value == '\0') {
                    // sentinel — apply default color but don't advance global index
                    ch.color = defaultColor;
                    ch.style = FontStyle::Normal;
                    continue;
                }
                int globalIdx = lineOffset + ch.index;
                if (globalIdx < (int)m_formats.size()) {
                    ch.color = m_formats[globalIdx].color;
                    ch.style = m_formats[globalIdx].fontStyle;
                } else {
                    ch.color = defaultColor;
                    ch.style = FontStyle::Normal;
                }
            }
            lineOffset += (int)line.chars.size();
        }

        // Compute total text size
        if (!m_lines.empty()) {
            text::ApplyTextStyle(g, textStyle);
            int lineHeight = static_cast<int>(g.GetFontExtents().height);
            int maxW = 0;
            for (const auto& line : m_lines)
                maxW = std::max(maxW, line.bounds.w);
            m_textSize = {maxW, lineHeight * (int)m_lines.size()};
        } else {
            m_textSize = {0, 0};
        }
    }

    // -------------------------------------------------------------------------
    // ComputeLineStarts
    // Returns the global m_textRaw index at which each line begins.
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

    // -------------------------------------------------------------------------
    // GetCaretLineInfo
    // -------------------------------------------------------------------------
    TextArea::LineInfo TextArea::GetCaretLineInfo() const {
        auto starts = ComputeLineStarts();
        int lineIdx = 0;
        for (int i = (int)starts.size() - 1; i >= 0; --i) {
            if (m_caretIndex >= starts[i]) {
                lineIdx = i;
                break;
            }
        }
        int lineStart = starts[lineIdx];
        int lineLen = (int)m_lines[lineIdx].chars.size() - 1; // exclude sentinel
        return {lineIdx, lineStart, lineLen};
    }

    // -------------------------------------------------------------------------
    // OnDraw
    // -------------------------------------------------------------------------
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

        auto textStyle = GetStyle()["DefaultText"];
        text::ApplyTextStyle(g, textStyle);
        auto fm = g.GetFontExtents();
        int lineHeight = static_cast<int>(fm.height);

        EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
        int visW = sz.w - pad.left - pad.right;
        int visH = sz.h - pad.top - pad.bottom;

        g.ClipPushRect(pad.left, pad.top, visW, visH);
        g.Translate(pad.left, pad.top);

        // --- Selection background (multi-line aware) ---
        if (IsSelected()) {
            int selA = m_selectionStart, selB = m_selectionEnd;
            if (selA > selB)
                std::swap(selA, selB);

            auto starts = ComputeLineStarts();
            for (int li = 0; li < (int)m_lines.size(); ++li) {
                const auto& line = m_lines[li];
                int lineStart = starts[li];
                int lineEnd = lineStart + (int)line.chars.size() - 1; // before sentinel

                // Does the selection overlap this line?
                if (selB <= lineStart || selA >= lineStart + (int)line.chars.size())
                    continue;

                int localA = std::max(0, selA - lineStart);
                int localB = std::min((int)line.chars.size() - 1, selB - lineStart);
                localA = std::min(localA, (int)line.chars.size() - 1);
                localB = std::min(localB, (int)line.chars.size() - 1);

                int x0 = line.chars[localA].bounds.x;
                int x1 = line.chars[localB].bounds.x;
                if (x1 <= x0 && localB > localA)
                    x1 = x0 + 4; // ensure at least a sliver

                int lineY = li * lineHeight;
                g.BeginPath();
                g.AddPathRect(x0, lineY, x1 - x0, lineHeight);
                g.EndPath(true);
                g.StyledPaint(GetStyle()["Selection"]);
            }
        }

        // --- Text ---
        const std::string fontName = textStyle.value("font", "Sans");
        const double fontSize = textStyle.value("fontSize", 14.0);
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
        if (IsFocused() && m_editable && !m_lines.empty()) {
            auto info = GetCaretLineInfo();
            const auto& line = m_lines[info.lineIdx];
            int localCaretIdx = m_caretIndex - info.lineStart;
            localCaretIdx = std::max(0, std::min(localCaretIdx, (int)line.chars.size() - 1));
            int caretX = line.chars[localCaretIdx].bounds.x;
            int caretY = info.lineIdx * lineHeight;

            Color caretColor = Color::FromStyle(textStyle["color"]);
            g.Color(caretColor.r, caretColor.g, caretColor.b, 1.0f);
            g.LineWidth(1.0f);
            g.Line(caretX, caretY, caretX, caretY + lineHeight + 1);
            g.Stroke();
        }

        g.ClipPop();
    }

    // -------------------------------------------------------------------------
    // OnKeyDown
    // -------------------------------------------------------------------------
    void TextArea::OnKeyDown(KeyEvent e) {
        if (!m_editable)
            return;

        if (e.key == Key::Enter) {
            if (IsSelected())
                DeleteSelected();
            InsertChar('\n');
            Invalidate();
            return;
        }

        if (e.key == Key::Up) {
            auto info = GetCaretLineInfo();
            if (info.lineIdx > 0) {
                auto starts = ComputeLineStarts();
                int localCol = m_caretIndex - info.lineStart;
                int prevLineStart = starts[info.lineIdx - 1];
                int prevLineLen = (int)m_lines[info.lineIdx - 1].chars.size() - 1;
                m_caretIndex = prevLineStart + std::min(localCol, prevLineLen);
            }
            Invalidate();
            return;
        }

        if (e.key == Key::Down) {
            auto info = GetCaretLineInfo();
            if (info.lineIdx < (int)m_lines.size() - 1) {
                auto starts = ComputeLineStarts();
                int localCol = m_caretIndex - info.lineStart;
                int nextLineStart = starts[info.lineIdx + 1];
                int nextLineLen = (int)m_lines[info.lineIdx + 1].chars.size() - 1;
                m_caretIndex = nextLineStart + std::min(localCol, nextLineLen);
            }
            Invalidate();
            return;
        }

        if (e.key == Key::Home) {
            auto info = GetCaretLineInfo();
            m_caretIndex = info.lineStart;
            Invalidate();
            return;
        }

        if (e.key == Key::End) {
            auto info = GetCaretLineInfo();
            m_caretIndex = info.lineStart + info.lineLen;
            Invalidate();
            return;
        }

        LineEdit::OnKeyDown(e);
    }

    // -------------------------------------------------------------------------
    // GetPreferredSize
    // -------------------------------------------------------------------------
    Size TextArea::GetPreferredSize() const {
        Size sz = GetSize();
        if (m_textSize.w > 0 || m_textSize.h > 0) {
            EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
            return {
                m_textSize.w + (int)pad.left + (int)pad.right,
                m_textSize.h + (int)pad.top + (int)pad.bottom
            };
        }
        return sz;
    }

    // -------------------------------------------------------------------------
    // Format
    // -------------------------------------------------------------------------
    void TextArea::Format(int from, int len, FontStyle style, float r, float g, float b) {
        int fmtSize = (int)m_formats.size();
        from = std::max(0, std::min(from, fmtSize));
        int to = std::max(0, std::min(from + len, fmtSize));
        for (int i = from; i < to; ++i) {
            m_formats[i].color = Color::FromRGB(r, g, b);
            m_formats[i].fontStyle = style;
        }
        Rebuild();
        Invalidate();
    }

    void TextArea::Format(FontStyle style, float r, float g, float b) {
        if (!IsSelected())
            return;
        int selA = m_selectionStart, selB = m_selectionEnd;
        if (selA > selB)
            std::swap(selA, selB);
        Format(selA, selB - selA, style, r, g, b);
    }

} // namespace gui
