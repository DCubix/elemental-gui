#include "FlexLayout.h"

#include "Element.h"

#include <algorithm>

namespace gui {

    FlexLayout::FlexLayout(FlexDirection dir, FlexJustify justify, FlexAlign align)
        : m_direction(dir),
          m_justify(justify),
          m_align(align) {}

    void FlexLayout::Add(Element* element) {
        m_items.push_back(element);
    }

    void FlexLayout::Remove(Element* element) {
        m_items.erase(std::remove(m_items.begin(), m_items.end(), element), m_items.end());
    }

    void FlexLayout::Apply(const Rectangle& bounds) {
        if (m_items.empty())
            return;

        int mainPadding = (m_direction == FlexDirection::Row) ? m_padding.GetHorizontal()
                                                              : m_padding.GetVertical();
        int crossPadding = (m_direction == FlexDirection::Row) ? m_padding.GetVertical()
                                                               : m_padding.GetHorizontal();
        int availMain = (m_direction == FlexDirection::Row ? bounds.w : bounds.h) - mainPadding;
        int availCross = (m_direction == FlexDirection::Row ? bounds.h : bounds.w) - crossPadding;

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
            if (!item->IsVisible())
                continue;

            Size s = item->GetPreferredSize();
            int ms = (m_direction == FlexDirection::Row) ? s.w : s.h;
            int cs = (m_direction == FlexDirection::Row) ? s.h : s.w;

            visible.push_back({item, ms, cs, item->GetFlexGrow()});

            if (item->GetFlexGrow() <= 0)
                totalFixedMain += ms;

            totalGrow += item->GetFlexGrow();
        }

        if (visible.empty())
            return;
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
        for (auto& info : visible)
            totalMain += info.mainSize;

        int freeSpace = std::max(0, availMain - totalMain - totalGaps);

        const float startPos =
            static_cast<float>(m_direction == FlexDirection::Row ? m_padding.left : m_padding.top);

        float mainPos = startPos;
        float itemGap = static_cast<float>(m_gap);

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
                if (n > 1)
                    itemGap += (float)freeSpace / (n - 1);
                break;
            case FlexJustify::SpaceEvenly: {
                float space = (float)(availMain - totalMain) / (n + 1);
                mainPos = startPos + space;
                itemGap = space;
                break;
            }
        }

        for (auto& info : visible) {
            int crossPos = m_direction == FlexDirection::Row ? m_padding.top : m_padding.left;
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

            Rectangle newBounds{};
            if (m_direction == FlexDirection::Row) {
                newBounds = Rectangle((int)mainPos, crossPos, info.mainSize, crossSize);
            } else {
                newBounds = Rectangle(crossPos, (int)mainPos, crossSize, info.mainSize);
            }
            info.elem->SetLocalBounds(newBounds);

            mainPos += info.mainSize + itemGap;
        }
    }

    Size FlexLayout::GetLaidOutSize() const {
        int mainPad = (m_direction == FlexDirection::Row) ? m_padding.GetHorizontal()
                                                          : m_padding.GetVertical();
        int crossPad = (m_direction == FlexDirection::Row) ? m_padding.GetVertical()
                                                           : m_padding.GetHorizontal();

        int mainTotal = 0;
        int crossMax = 0;
        int visibleCount = 0;

        for (auto& e : m_items) {
            if (!e || !e->IsVisible())
                continue;

            Size s = e->GetPreferredSize();
            int ms = (m_direction == FlexDirection::Row) ? s.w : s.h;
            int cs = (m_direction == FlexDirection::Row) ? s.h : s.w;

            mainTotal += ms;
            if (cs > crossMax)
                crossMax = cs;
            visibleCount++;
        }

        if (visibleCount > 1)
            mainTotal += (visibleCount - 1) * m_gap;
        mainTotal += mainPad;
        crossMax += crossPad;

        if (m_direction == FlexDirection::Row)
            return {mainTotal, crossMax};
        else
            return {crossMax, mainTotal};
    }
} // namespace gui
