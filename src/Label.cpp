#include "Label.h"

#include "Window.h"

#include <algorithm>

namespace gui {

    Label::Label()
        : Element() {
        TrackAll(text, alignment, iconSize, icon);
    }

    void Label::OnDraw(Graphics& g) {
        auto style = GetStyle();
        if (m_textStyle.is_object())
            style.update(m_textStyle);

        auto sz = GetSize();
        Rectangle clip = GetLocalIntersectedBounds();

        g.Save();
        g.StyledTextBegin(style);
        g.ClipPushRect(clip.x, clip.y, clip.w, clip.h);

        int iconSz = icon() ? (int)iconSize() : 0;
        bool hasText = !text().empty();

        // Icon-only: always center+middle
        if (icon() && !hasText) {
            g.DrawImage(icon(), sz.w / 2 - iconSz / 2, sz.h / 2 - iconSz / 2, iconSz, iconSz);
            g.ClipPop();
            g.Restore();
            return;
        }

        auto&& lines = utils::SplitString(text(), "\n");

        auto fm = g.GetFontExtents();
        int lineH = static_cast<int>(fm.ascent + fm.descent);

        std::string font = "Sans";
        double fontSize = 14.0;
        if (style["font"].is_string())
            font = style["font"].get<std::string>();
        if (style["fontSize"].is_number())
            fontSize = style["fontSize"].get<double>();

        int maxW = 0;
        std::vector<int> lineWidths;
        lineWidths.reserve(lines.size());
        for (auto& line : lines) {
            int lineW = 0;
            for (const auto& md : utils::ParseBasicMarkdown(line)) {
                FontStyle fStyle = FontStyle::Normal;
                if (md.bold && md.italic)
                    fStyle = FontStyle::BoldItalic;
                else if (md.bold)
                    fStyle = FontStyle::Bold;
                else if (md.italic)
                    fStyle = FontStyle::Italic;
                g.Font(fStyle, font, fontSize);
                lineW += static_cast<int>(g.MeasureText(md.text).xAdvance);
            }
            lineWidths.push_back(lineW);
            maxW = std::max(maxW, lineW);
        }

        int textBlockH = lineH * (int)lines.size();
        int gap = icon() ? 4 : 0;
        int totalW = iconSz + gap + maxW;
        int totalH = std::max(iconSz, textBlockH);

        // Content block origin based on alignment
        int cx = 0, cy = 0;
        switch (alignment()) {
            case Alignment::TopCenter:
            case Alignment::MiddleCenter:
            case Alignment::BottomCenter:
                cx = sz.w / 2 - totalW / 2;
                break;
            case Alignment::TopRight:
            case Alignment::MiddleRight:
            case Alignment::BottomRight:
                cx = sz.w - totalW;
                break;
            default:
                break;
        }
        switch (alignment()) {
            case Alignment::MiddleLeft:
            case Alignment::MiddleCenter:
            case Alignment::MiddleRight:
                cy = sz.h / 2 - totalH / 2;
                break;
            case Alignment::BottomLeft:
            case Alignment::BottomCenter:
            case Alignment::BottomRight:
                cy = sz.h - totalH;
                break;
            default:
                break;
        }

        if (icon()) {
            g.DrawImage(icon(), cx, cy + totalH / 2 - iconSz / 2, iconSz, iconSz);
        }

        int textX = cx + iconSz + gap;
        // baseline of first line: top of text block + ascent
        int baselineY = cy + (totalH - textBlockH) / 2 + static_cast<int>(fm.ascent);

        for (size_t i = 0; i < lines.size(); i++) {
            int lineX = textX;
            switch (alignment()) {
                case Alignment::TopCenter:
                case Alignment::MiddleCenter:
                case Alignment::BottomCenter:
                    lineX += maxW / 2 - lineWidths[i] / 2;
                    break;
                case Alignment::TopRight:
                case Alignment::MiddleRight:
                case Alignment::BottomRight:
                    lineX += maxW - lineWidths[i];
                    break;
                default:
                    break;
            }

            int relX = lineX;
            for (const auto& md : utils::ParseBasicMarkdown(lines[i])) {
                FontStyle fStyle = FontStyle::Normal;
                if (md.bold && md.italic)
                    fStyle = FontStyle::BoldItalic;
                else if (md.bold)
                    fStyle = FontStyle::Bold;
                else if (md.italic)
                    fStyle = FontStyle::Italic;

                g.Font(fStyle, font, fontSize);
                auto segEx = g.MeasureText(md.text);
                g.StyledTextEnd(md.text, relX, baselineY);
                relX += static_cast<int>(segEx.xAdvance);
            }
            baselineY += lineH;
        }

        g.ClipPop();
        g.Restore();
    }

    Size Label::GetPreferredSize() const {
        if (IsAutoSize()) {
            int iconSz = icon() ? (int)iconSize() : 0;

            if (text().empty()) {
                return {iconSz, iconSz};
            }

            const auto textStyle = GetStyle();
            auto& g = m_window->GetGraphics();

            g.Save();
            g.StyledTextBegin(textStyle);

            std::string font = "Sans";
            double fontSize = 14.0;
            if (textStyle["font"].is_string())
                font = textStyle["font"].get<std::string>();
            if (textStyle["fontSize"].is_number())
                fontSize = textStyle["fontSize"].get<double>();

            auto fm = g.GetFontExtents();
            int lineH = static_cast<int>(fm.ascent + fm.descent);
            auto lines = utils::SplitString(text(), "\n");
            int maxW = 0;
            for (auto& line : lines) {
                int lineW = 0;
                for (const auto& md : utils::ParseBasicMarkdown(line)) {
                    FontStyle fStyle = FontStyle::Normal;
                    if (md.bold && md.italic)
                        fStyle = FontStyle::BoldItalic;
                    else if (md.bold)
                        fStyle = FontStyle::Bold;
                    else if (md.italic)
                        fStyle = FontStyle::Italic;
                    g.Font(fStyle, font, fontSize);
                    lineW += static_cast<int>(g.MeasureText(md.text).xAdvance);
                }
                maxW = std::max(maxW, lineW);
            }
            int textH = lineH * (int)lines.size();

            g.Restore();

            int gap = icon() ? 4 : 0;
            return {iconSz + gap + maxW, std::max(iconSz, textH)};
        }
        return Element::GetPreferredSize();
    }

    namespace utils {
        std::vector<MarkdownToken> ParseBasicMarkdown(const std::string& text) {
            std::vector<MarkdownToken> tokens;
            std::vector<char> input(text.begin(), text.end());

            auto fnPop = [&input]() {
                char tmp = input.front();
                input.erase(input.begin());
                return tmp;
            };

            auto fnPeek = [&input]() {
                return input.empty() ? '\0' : input.front();
            };

            auto fnFlush = [&tokens](std::string& buffer, bool bold, bool italic) {
                if (!buffer.empty()) {
                    tokens.push_back({bold, italic, buffer});
                    buffer.clear();
                }
            };

            std::string buffer;
            bool bold = false, italic = false;

            while (!input.empty()) {
                char c = fnPop();

                if (c == '*') {
                    if (fnPeek() == '*') {
                        fnPop();
                        buffer += "**";
                    } else {
                        fnFlush(buffer, bold, italic);
                        bold = !bold;
                    }
                } else if (c == '_') {
                    fnFlush(buffer, bold, italic);
                    italic = !italic;
                } else {
                    buffer += c;
                }
            }

            fnFlush(buffer, bold, italic);

            return tokens;
        }

        std::string CleanMarkdown(const std::string& text) {
            std::string buffer;
            std::vector<char> input(text.begin(), text.end());

            while (!input.empty()) {
                char c = input.front();
                input.erase(input.begin());

                if (c == '*' || c == '_')
                    continue;
                buffer += c;
            }

            return buffer;
        }
    } // namespace utils
} // namespace gui
