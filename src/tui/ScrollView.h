#ifndef TUI_SCROLL_VIEW_H
#define TUI_SCROLL_VIEW_H

#include "Scrollbar.h"
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
		Scrollbar *m_verticalScrollbar, *m_horizontalScrollbar;
		Element *m_element;

		bool IsDirty() override;
	};
}

#endif // TUI_SCROLL_VIEW_H
