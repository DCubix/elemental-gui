#include "ImageView.h"

namespace tui {

	ImageView::ImageView()
		: m_image(nullptr)
	{}

	void ImageView::OnDraw(Graphics& g) {
		if (!m_image) return;

		Rectangle b = GetBounds();
		Rectangle c = GetIntersectedBounds();
		Size sz = GetPreferredSize();

		Rectangle imgRect{ b.x, b.y, sz.w, sz.h };
		switch (m_scalingMode) {
			case ImageScalingMode::Stretch:
				break;
			case ImageScalingMode::Contain: {
				float imgAspect = static_cast<float>(m_image->GetWidth()) / static_cast<float>(m_image->GetHeight());
				float boxAspect = static_cast<float>(b.w) / static_cast<float>(b.h);
				if (imgAspect > boxAspect) {
					imgRect.w = b.w;
					imgRect.h = static_cast<int>(b.w / imgAspect);
					imgRect.y += (b.h - imgRect.h) / 2;
				} else {
					imgRect.h = b.h;
					imgRect.w = static_cast<int>(b.h * imgAspect);
					imgRect.x += (b.w - imgRect.w) / 2;
				}
				break;
			}
			case ImageScalingMode::Cover: {
				float imgAspect = static_cast<float>(m_image->GetWidth()) / static_cast<float>(m_image->GetHeight());
				float boxAspect = static_cast<float>(b.w) / static_cast<float>(b.h);
				if (imgAspect < boxAspect) {
					imgRect.w = b.w;
					imgRect.h = static_cast<int>(b.w / imgAspect);
					imgRect.y += (b.h - imgRect.h) / 2;
				} else {
					imgRect.h = b.h;
					imgRect.w = static_cast<int>(b.h * imgAspect);
					imgRect.x += (b.w - imgRect.w) / 2;
				}
				break;
			}
		}

		g.ClipPushRect(c.x, c.y, c.w, c.h);
		g.DrawImage(m_image.get(), imgRect.x, imgRect.y, imgRect.w, imgRect.h);
		g.ClipPop();
	}

	Size ImageView::GetPreferredSize() const {
		if (m_image && IsAutoSize()) {
			return { m_image->GetWidth(), m_image->GetHeight() };
		}
		return { GetLocalBounds().w, GetLocalBounds().h };
	}

}
