#include "FlexLayout.h"
#include "Element.h"

#include <algorithm>

namespace tui {

	FlexLayout::FlexLayout(FlexDirection dir, FlexJustify justify, FlexAlign align)
		: m_direction(dir), m_justify(justify), m_align(align)
	{}

	void FlexLayout::Add(Element* element) {
		m_items.push_back(element);
	}

	void FlexLayout::Apply(int x, int y, int w, int h) {
		if (m_items.empty()) return;

		int availMain = (m_direction == FlexDirection::Row ? w : h) - 2 * m_padding;
		int availCross = (m_direction == FlexDirection::Row ? h : w) - 2 * m_padding;

		struct ItemInfo {
			Element* elem;
			int mainSize;
			int crossSize;
			float grow;
		};

		std::vector<ItemInfo> visible;
		int totalFixedMain = 0;
		float totalGrow = 0;

		for (auto& item : m_items) {
			if (!item->IsVisible()) continue;
			Size s = item->GetPreferredSize();
			int ms = (m_direction == FlexDirection::Row) ? s.w : s.h;
			int cs = (m_direction == FlexDirection::Row) ? s.h : s.w;
			visible.push_back({item, ms, cs, item->GetFlexGrow()});
			if (item->GetFlexGrow() <= 0) totalFixedMain += ms;
			totalGrow += item->GetFlexGrow();
		}

		if (visible.empty()) return;
		int n = (int)visible.size();

		int totalGaps = (n - 1) * m_gap;
		int spaceForGrow = availMain - totalFixedMain - totalGaps;

		// Distribute space to growing items
		if (totalGrow > 0 && spaceForGrow > 0) {
			for (auto& info : visible) {
				if (info.grow > 0) {
					info.mainSize = (int)(spaceForGrow * info.grow / totalGrow);
				}
			}
		}

		// Recalculate total main size after grow distribution
		int totalMain = 0;
		for (auto& info : visible) totalMain += info.mainSize;

		int freeSpace = std::max(0, availMain - totalMain - totalGaps);

		float mainPos = (float)m_padding;
		float itemGap = (float)m_gap;

		switch (m_justify) {
			case FlexJustify::Start:
				break;
			case FlexJustify::End:
				mainPos += freeSpace;
				break;
			case FlexJustify::Center:
				mainPos += freeSpace / 2.0f;
				break;
			case FlexJustify::SpaceBetween:
				if (n > 1) itemGap += (float)freeSpace / (n - 1);
				break;
			case FlexJustify::SpaceEvenly: {
				float space = (float)(availMain - totalMain) / (n + 1);
				mainPos = m_padding + space;
				itemGap = space;
				break;
			}
		}

		for (auto& info : visible) {
			int crossPos = m_padding;
			int crossSize = info.crossSize;

			switch (m_align) {
				case FlexAlign::Start:
					break;
				case FlexAlign::Center:
					crossPos += (availCross - crossSize) / 2;
					break;
				case FlexAlign::End:
					crossPos += availCross - crossSize;
					break;
				case FlexAlign::Stretch:
					crossSize = availCross;
					break;
			}

			if (m_direction == FlexDirection::Row) {
				info.elem->GetLocalBounds() = Rectangle((int)mainPos, crossPos, info.mainSize, crossSize);
			} else {
				info.elem->GetLocalBounds() = Rectangle(crossPos, (int)mainPos, crossSize, info.mainSize);
			}

			mainPos += info.mainSize + itemGap;
		}
	}

}
