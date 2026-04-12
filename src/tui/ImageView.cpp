#include "ImageView.h"

namespace tui {

	ImageView::ImageView()
		: m_image(nullptr)
	{}

	void ImageView::OnDraw(Graphics& g) {
		Rectangle b = GetBounds();
		Rectangle c = GetIntersectedBounds();
		Size sz = GetPreferredSize();

		if (m_image != nullptr) {
			g.ClipPush(c.x, c.y, c.w, c.h);
			g.DrawImage(m_image, b.x, b.y, sz.w, sz.h);
			g.ClipPop();
		}
	}

	Size ImageView::GetPreferredSize() {
		if (m_image != nullptr && IsAutoSize()) {
			return { m_image->GetWidth(), m_image->GetHeight() };
		}
		return { GetLocalBounds().w, GetLocalBounds().h };
	}

}
