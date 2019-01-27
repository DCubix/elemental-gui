#include "ImageView.h"

namespace tui {

	ImageView::ImageView()
		: m_image(nullptr)
	{}

	void ImageView::onDraw(Graphics& g) {
		Rect b = bounds();
		Rect c = intersectedBounds();
		Size sz = preferredSize();

		if (m_image != nullptr) {
			g.clipPush(c.x, c.y, c.w, c.h);
			g.image(m_image, b.x, b.y, sz.w, sz.h);
			g.clipPop();
		}
	}

	Size ImageView::preferredSize() {
		if (m_image != nullptr && autoSize()) {
			return { m_image->width(), m_image->height() };
		}
		return { localBounds().w, localBounds().h };
	}

}
