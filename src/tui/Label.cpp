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

	void Label::onDraw(Graphics& g) {
		if (m_style.is_null()) {
			m_style = app()->style()["DefaultText"];
		}

		Rect b = bounds();
		Rect c = intersectedBounds();

		g.save();
		g.styledTextBegin(m_style);

		g.clipPush(c.x, c.y, c.w, c.h);
		int offx = 0;
		int maxW = 0, maxH = 0;

		auto&& lines = utils::splitString(m_text, "\n");
		for (auto&& text : lines) {
			auto&& ex = g.measureText(text);
			maxH += ex.height;
			maxW = std::max(maxW, int(ex.width));
		}

		if (m_icon != nullptr) {
			int th = maxH;
			int ix = b.x;
			int iy = b.y;
			int ty = 0;
			int midY = (th / 2 - m_icon->height() / 2);
			int midY2 = (th / 2 + m_icon->height() / 2);
			int iwo = m_icon->width() + 4;
			switch (m_alignment) {
				case TopLeft:
					offx += iwo;
					ty = midY;
					break;
				case MiddleLeft:
					offx += iwo;
					ty = b.h / 2 - midY;
					break;
				case BottomLeft:
					offx += iwo;
					ty = b.h - midY2;
					break;

				case TopCenter:
					ix += b.w / 2 - (maxW / 2 + iwo);
					ty = midY;
					break;
				case MiddleCenter:
					ix += b.w / 2 - (maxW / 2 + iwo);
					ty = b.h / 2 - midY;
					break;
				case BottomCenter:
					ix += b.w / 2 - (maxW / 2 + iwo);
					ty = b.h - midY2;
					break;

				case TopRight:
					offx -= iwo + 4;
					ix += b.w - iwo;
					ty = midY;
					break;
				case MiddleRight:
					offx -= iwo + 4;
					ix += b.w - iwo;
					ty = b.h / 2 - midY;
					break;
				case BottomRight:
					offx -= iwo + 4;
					ix += b.w - iwo;
					ty = b.h - midY2;
					break;
			}
			g.image(m_icon, ix, iy + ty, m_icon->width(), m_icon->height());
		}

		int offy = 0;
		switch (m_alignment) {
			case TopLeft:
			case TopCenter:
			case TopRight: offy = 0; break;
			case MiddleLeft:
			case MiddleCenter:
			case MiddleRight: offy = b.h / 2 - maxH / 2; break;
			case BottomLeft:
			case BottomCenter:
			case BottomRight: offy = b.h - maxH; break;
		}

		for (auto&& text : lines) {
			auto&& ex = g.measureText(text);
			double hx = (ex.width / 2);

			int tx = b.x;
			int ty = b.y;
			switch (m_alignment) {
				case TopLeft: break;
				case TopCenter: tx += b.w / 2 - hx; break;
				case TopRight: tx += b.w - hx*2; break;
				case MiddleLeft: break;
				case MiddleCenter: tx += b.w / 2 - hx; break;
				case MiddleRight: tx += b.w - hx*2; break;
				case BottomLeft: break;
				case BottomCenter: tx += b.w / 2 - hx; break;
				case BottomRight: tx += b.w - hx*2; break;
			}
			g.styledTextEnd(text, tx + offx, ty + ex.height + offy);
			offy += ex.height;
		}
		g.clipPop();
		g.restore();
	}

}
