#pragma once

#include "Layout.h"
#include <vector>

namespace tui {
	class Element;

	enum class FlexDirection { Row, Column };
	enum class FlexJustify { Start, Center, End, SpaceBetween, SpaceEvenly };
	enum class FlexAlign { Start, Center, End, Stretch };

	class FlexLayout : public Layout {
	public:
		FlexLayout(FlexDirection dir = FlexDirection::Row,
				   FlexJustify justify = FlexJustify::Start,
				   FlexAlign align = FlexAlign::Stretch);

		void Add(Element* element);
		void Apply(int x, int y, int w, int h) override;

		void SetDirection(FlexDirection dir) { m_direction = dir; }
		FlexDirection GetDirection() const { return m_direction; }

		void SetJustifyContent(FlexJustify justify) { m_justify = justify; }
		FlexJustify GetJustifyContent() const { return m_justify; }

		void SetAlignItems(FlexAlign align) { m_align = align; }
		FlexAlign GetAlignItems() const { return m_align; }

	private:
		FlexDirection m_direction;
		FlexJustify m_justify;
		FlexAlign m_align;
		std::vector<Element*> m_items;
	};
}
