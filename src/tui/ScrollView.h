#ifndef TUI_SCROLL_VIEW_H
#define TUI_SCROLL_VIEW_H

#include "Slider.h"
#include "Element.h"

namespace tui {
	class ScrollView : public Element {
	public:
		ScrollView();

		EventStatus OnEvent(Event *event) override;
		void OnDraw(Graphics& g) override;

		void SetElement(Element *e);
		Element* GetElement() { return m_element; }

	private:
		Slider *m_verticalSlider, *m_horizontalSlider;
		Element *m_element;

		bool IsDirty() override;
	};
}

#endif // TUI_SCROLL_VIEW_H
