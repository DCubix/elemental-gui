#pragma once

#include "Element.h"

namespace tui {
	class ImageView : public Element {
	public:
		ImageView();

		Image* GetImage() { return m_image; }
		void SetImage(Image *img) { m_image = img; Invalidate(); }

		void OnDraw(Graphics& g) override;
		Size GetPreferredSize() override;

	private:
		Image *m_image;
	};
}
