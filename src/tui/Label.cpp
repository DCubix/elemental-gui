#include "Label.h"

#include "Application.h"

#include <vector>
#include <algorithm>

namespace tui {

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

		g.ClipPush(c.x, c.y, c.w, c.h);
		int offx = 0;
		int maxW = 0, maxH = 0;

		auto&& lines = utils::SplitString(m_text, "\n");
		for (auto&& text : lines) {
			auto&& ex = g.MeasureText(text);
			maxH += ex.height;
			maxW = std::max(maxW, int(ex.width));
		}

		if (m_icon != nullptr) {
			int th = maxH;
			int ix = b.x;
			int iy = b.y;
			int ty = 0;
			int midY = (th / 2 - m_icon->GetHeight() / 2);
			int midY2 = (th / 2 + m_icon->GetHeight() / 2);
			int iwo = m_icon->GetWidth() + 4;
			switch (m_alignment) {
				case Alignment::TopLeft:
					offx += iwo;
					ty = midY;
					break;
				case Alignment::MiddleLeft:
					offx += iwo;
					ty = b.h / 2 - midY;
					break;
				case Alignment::BottomLeft:
					offx += iwo;
					ty = b.h - midY2;
					break;

				case Alignment::TopCenter:
					ix += b.w / 2 - (maxW / 2 + iwo);
					ty = midY;
					break;
				case Alignment::MiddleCenter:
					ix += b.w / 2 - (maxW / 2 + iwo);
					ty = b.h / 2 - midY;
					break;
				case Alignment::BottomCenter:
					ix += b.w / 2 - (maxW / 2 + iwo);
					ty = b.h - midY2;
					break;

				case Alignment::TopRight:
					offx -= iwo + 4;
					ix += b.w - iwo;
					ty = midY;
					break;
				case Alignment::MiddleRight:
					offx -= iwo + 4;
					ix += b.w - iwo;
					ty = b.h / 2 - midY;
					break;
				case Alignment::BottomRight:
					offx -= iwo + 4;
					ix += b.w - iwo;
					ty = b.h - midY2;
					break;
			}
			g.DrawImage(m_icon, ix, iy + ty, m_icon->GetWidth(), m_icon->GetHeight());
		}

		int offy = 0;
		switch (m_alignment) {
			case Alignment::TopLeft:
			case Alignment::TopCenter:
			case Alignment::TopRight: offy = 0; break;
			case Alignment::MiddleLeft:
			case Alignment::MiddleCenter:
			case Alignment::MiddleRight: offy = b.h / 2 - maxH / 2; break;
			case Alignment::BottomLeft:
			case Alignment::BottomCenter:
			case Alignment::BottomRight: offy = b.h - maxH; break;
		}

		for (auto&& text : lines) {
			auto&& ex = g.MeasureText(text);
			double hx = (ex.width / 2);

			int tx = b.x;
			int ty = b.y;
			switch (m_alignment) {
				case Alignment::TopLeft: break;
				case Alignment::TopCenter: tx += b.w / 2 - hx; break;
				case Alignment::TopRight: tx += b.w - hx*2; break;
				case Alignment::MiddleLeft: break;
				case Alignment::MiddleCenter: tx += b.w / 2 - hx; break;
				case Alignment::MiddleRight: tx += b.w - hx*2; break;
				case Alignment::BottomLeft: break;
				case Alignment::BottomCenter: tx += b.w / 2 - hx; break;
				case Alignment::BottomRight: tx += b.w - hx*2; break;
			}
			g.StyledTextEnd(text, tx + offx, ty + ex.height + offy);
			offy += ex.height;
		}
		g.ClipPop();
		g.Restore();
	}

}
