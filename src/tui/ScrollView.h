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

		Direction GetScrollDirection() const { return m_scrollDirection; }
		void SetScrollDirection(Direction dir);

	private:
		Direction m_scrollDirection;
		Scrollbar *m_scrollBar;
		Element *m_element;

		bool IsDirty() override;

		void SolveScrollVertical();
		void SolveScrollHorizontal();
	};
}
