#ifndef TUI_SCROLL_VIEW_H
#define TUI_SCROLL_VIEW_H

#include "Slider.h"
#include "Element.h"

namespace tui {
	class ScrollView : public Element {
	public:
		ScrollView();

		EventStatus onEvent(Event *event) override;
		void onDraw(Graphics& g) override;

		void element(Element *e);
		Element* element() { return m_element; }

	private:
		Slider *m_verticalSlider, *m_horizontalSlider;
		Element *m_element;

		bool dirty() override;
	};
}

#endif // TUI_SCROLL_VIEW_H
