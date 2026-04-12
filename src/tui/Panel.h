#ifndef TUI_PANEL_H
#define TUI_PANEL_H

#include "Element.h"

namespace tui {
	class Panel : public Element {
	public:
		Panel();

		void Add(Element *element, Layout::LayoutDirection dir);

		void OnDraw(Graphics& g) override;
//		EventStatus onEvent(Event *event) override;

		Layout& GetLayout() { return m_layout; }
		std::vector<Element*> GetChildren() { return m_children; }

		bool IsBackgroundVisible() const { return m_backgroundVisible; }
		void SetBackgroundVisible(bool bv) { m_backgroundVisible = bv; Invalidate(); }

	private:
		Layout m_layout;
		std::vector<Element*> m_children;
		bool m_backgroundVisible;

		bool IsDirty() override;
	};
}

#endif // TUI_PANEL_H
