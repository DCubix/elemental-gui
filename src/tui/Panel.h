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

		Layout& layout() { return m_layout; }
		std::vector<Element*> children() { return m_children; }

		bool backgroundVisible() const { return m_backgroundVisible; }
		void backgroundVisible(bool bv) { m_backgroundVisible = bv; invalidate(); }

	private:
		Layout m_layout;
		std::vector<Element*> m_children;
		bool m_backgroundVisible;

		bool dirty() override;
	};
}

#endif // TUI_PANEL_H
