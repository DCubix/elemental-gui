#include "Label.h"

#include "Window.h"

#include <algorithm>

namespace gui {

    Label::Label()
        : Element() {
        text.SetOnUpdate([this]{ Invalidate(); });
        alignment.SetOnUpdate([this]{ Invalidate(); });
        iconSize.SetOnUpdate([this]{ Invalidate(); });
    }

    void Label::OnDraw(Graphics& g) {
        auto textStyle = GetStyle();
        if (m_textStyle.is_object())
            textStyle.update(m_textStyle);

        auto sz = GetSize();
        Rectangle clip = GetLocalIntersectedBounds();

        g.Save();
        g.StyledTextBegin(textStyle);
        g.ClipPushRect(clip.x, clip.y, clip.w, clip.h);

        int iconSz = m_icon ? (int)iconSize() : 0;
        bool hasText = !text().empty();

        // Icon-only: always center+middle
        if (m_icon && !hasText) {
            g.DrawImage(m_icon, sz.w / 2 - iconSz / 2, sz.h / 2 - iconSz / 2, iconSz, iconSz);
            g.ClipPop();
            g.Restore();
            return;
        }

        int maxW = 0, maxH = 0;
        auto&& lines = utils::SplitString(text(), "\n");
        for (auto&& line : lines) {
            auto&& ex = g.MeasureText(line);
            maxH += ex.size.h;
            maxW = std::max(maxW, static_cast<int>(ex.size.w));
        }

        int gap = m_icon ? 4 : 0;
        int totalW = iconSz + gap + maxW;
        int totalH = std::max(iconSz, maxH);

        // Content block origin based on alignment
        int cx = 0, cy = 0;
        switch (alignment()) {
            case Alignment::TopCenter:
            case Alignment::MiddleCenter:
            case Alignment::BottomCenter:
                cx += sz.w / 2 - totalW / 2;
                break;
            case Alignment::TopRight:
            case Alignment::MiddleRight:
            case Alignment::BottomRight:
                cx += sz.w - totalW;
                break;
            default:
                break;
        }
        switch (alignment()) {
            case Alignment::MiddleLeft:
            case Alignment::MiddleCenter:
            case Alignment::MiddleRight:
                cy += sz.h / 2 - totalH / 2;
                break;
            case Alignment::BottomLeft:
            case Alignment::BottomCenter:
            case Alignment::BottomRight:
                cy += sz.h - totalH;
                break;
            default:
                break;
        }

        if (m_icon) {
            g.DrawImage(m_icon, cx, cy + totalH / 2 - iconSz / 2, iconSz, iconSz);
        }

        int textX = cx + iconSz + gap;
        int textY = cy + totalH / 2 - maxH / 2;
        for (auto&& line : lines) {
            auto&& ex = g.MeasureText(line);
            int tx = textX;
            switch (alignment()) {
                case Alignment::TopCenter:
                case Alignment::MiddleCenter:
                case Alignment::BottomCenter:
                    tx += maxW / 2 - static_cast<int>(ex.size.w) / 2;
                    break;
                case Alignment::TopRight:
                case Alignment::MiddleRight:
                case Alignment::BottomRight:
                    tx += maxW - static_cast<int>(ex.size.w);
                    break;
                default:
                    break;
            }
            g.StyledTextEnd(line, tx, textY + ex.size.h);
            textY += ex.size.h;
        }

        g.ClipPop();
        g.Restore();
    }

    Size Label::GetPreferredSize() const {
        if (IsAutoSize()) {
            int iconSz = m_icon ? (int)iconSize() : 0;

            if (text().empty()) {
                return {iconSz, iconSz};
            }

            const auto textStyle = GetStyle();
            auto& g = m_window->GetGraphics();
            int maxW = 0, maxH = 0;

            g.Save();
            g.StyledTextBegin(textStyle);

            const auto fm = g.GetFontExtents();
            const int lineHeight = static_cast<int>(fm.ascent + fm.descent);
            auto&& lines = utils::SplitString(text(), "\n");
            for (auto&& line : lines) {
                auto&& ex = g.MeasureText(line);
                maxH += std::max(lineHeight, ex.size.h);
                maxW = std::max(maxW, static_cast<int>(ex.size.w));
            }

            g.Restore();

            int gap = m_icon ? 4 : 0;
            return {iconSz + gap + maxW, std::max(iconSz, maxH)};
        }
        return Element::GetPreferredSize();
    }
} // namespace gui
