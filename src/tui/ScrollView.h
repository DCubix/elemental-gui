#pragma once

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

		void SetVerticalScrollEnabled(bool enabled) { m_verticalEnabled = enabled; Invalidate(); }
		bool IsVerticalScrollEnabled() const { return m_verticalEnabled; }

		void SetHorizontalScrollEnabled(bool enabled) { m_horizontalEnabled = enabled; Invalidate(); }
		bool IsHorizontalScrollEnabled() const { return m_horizontalEnabled; }

	private:
		Scrollbar *m_verticalScrollbar, *m_horizontalScrollbar;
		Element *m_element;
		bool m_verticalEnabled{ true };
		bool m_horizontalEnabled{ true };

		bool IsDirty() override;
	};
}
