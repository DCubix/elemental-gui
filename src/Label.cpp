#include "Label.h"

#include "Window.h"

#include <algorithm>

namespace gui {

	Label::Label()
		: Element(),
		  m_text(""),
		  m_alignment(Alignment::TopLeft),
		  m_icon(nullptr)
	{}

	void Label::OnDraw(Graphics& g) {
		const auto textStyle = GetStyle()["DefaultText"];

		Rectangle b = GetBounds();
		Rectangle c = GetIntersectedBounds();

		g.Save();
		g.StyledTextBegin(textStyle);
		g.ClipPushRect(c.x, c.y, c.w, c.h);

		int iconSz = m_icon ? m_iconSize : 0;
		bool hasText = !m_text.empty();

		// Icon-only: always center+middle
		if (m_icon && !hasText) {
			g.DrawImage(m_icon, b.x + b.w / 2 - iconSz / 2, b.y + b.h / 2 - iconSz / 2, iconSz, iconSz);
			g.ClipPop();
			g.Restore();
			return;
		}

		int maxW = 0, maxH = 0;
		auto&& lines = utils::SplitString(m_text, "\n");
		for (auto&& text : lines) {
			auto&& ex = g.MeasureText(text);
			maxH += ex.height;
			maxW = std::max(maxW, static_cast<int>(ex.width));
		}

		int gap = m_icon ? 4 : 0;
		int totalW = iconSz + gap + maxW;
		int totalH = std::max(iconSz, maxH);

		// Content block origin based on alignment
		int cx = b.x, cy = b.y;
		switch (m_alignment) {
			case Alignment::TopCenter:
			case Alignment::MiddleCenter:
			case Alignment::BottomCenter:
				cx += b.w / 2 - totalW / 2; break;
			case Alignment::TopRight:
			case Alignment::MiddleRight:
			case Alignment::BottomRight:
				cx += b.w - totalW; break;
			default: break;
		}
		switch (m_alignment) {
			case Alignment::MiddleLeft:
			case Alignment::MiddleCenter:
			case Alignment::MiddleRight:
				cy += b.h / 2 - totalH / 2; break;
			case Alignment::BottomLeft:
			case Alignment::BottomCenter:
			case Alignment::BottomRight:
				cy += b.h - totalH; break;
			default: break;
		}

		if (m_icon) {
			g.DrawImage(m_icon, cx, cy + totalH / 2 - iconSz / 2, iconSz, iconSz);
		}

		int textX = cx + iconSz + gap;
		int textY = cy + totalH / 2 - maxH / 2;
		for (auto&& text : lines) {
			auto&& ex = g.MeasureText(text);
			int tx = textX;
			switch (m_alignment) {
				case Alignment::TopCenter:
				case Alignment::MiddleCenter:
				case Alignment::BottomCenter:
					tx += maxW / 2 - static_cast<int>(ex.width) / 2; break;
				case Alignment::TopRight:
				case Alignment::MiddleRight:
				case Alignment::BottomRight:
					tx += maxW - static_cast<int>(ex.width); break;
				default: break;
			}
			g.StyledTextEnd(text, tx, textY + ex.height);
			textY += ex.height;
		}

		g.ClipPop();
		g.Restore();
	}

    Size Label::GetPreferredSize() const
    {
		if (IsAutoSize()) {
			int iconSz = m_icon ? m_iconSize : 0;

			if (m_text.empty()) {
				return { iconSz, iconSz };
			}

			const auto textStyle = GetStyle()["DefaultText"];

			auto& g = m_window->GetGraphics();
			int maxW = 0, maxH = 0;

			g.Save();
			g.StyledTextBegin(textStyle);

			auto&& lines = utils::SplitString(m_text, "\n");
			for (auto&& text : lines) {
				auto&& ex = g.MeasureText(text);
				maxH += ex.height;
				maxW = std::max(maxW, static_cast<int>(ex.width));
			}

			g.Restore();

			int gap = m_icon ? 4 : 0;
        	return { iconSz + gap + maxW, std::max(iconSz, maxH) };
		}
		return Element::GetPreferredSize();
    }
}
