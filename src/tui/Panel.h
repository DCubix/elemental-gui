#ifndef TUI_PANEL_H
#define TUI_PANEL_H

#include "Element.h"

namespace tui {
	class Panel : public Element {
	public:
		Panel();

		void add(Element *element, Layout::LayoutDirection dir);

		void onDraw(Graphics& g) override;
//		EventStatus onEvent(Event *event) override;

	private:
		Layout m_layout;

		bool dirty() override;
	};
}

#endif // TUI_PANEL_H
