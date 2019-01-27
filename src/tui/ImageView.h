#ifndef TUI_IMAGE_VIEW_H
#define TUI_IMAGE_VIEW_H

#include "Element.h"

namespace tui {
	class ImageView : public Element {
	public:
		ImageView();

		Image* image() { return m_image; }
		void image(Image *img) { m_image = img; invalidate(); }

		void onDraw(Graphics& g) override;
		Size preferredSize() override;

	private:
		Image *m_image;
	};
}

#endif // TUI_IMAGE_VIEW_H
