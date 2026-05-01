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

        bool isRow = (m_direction == FlexDirection::Row);
        int mainPadStart  = isRow ? (int)m_padding.left   : (int)m_padding.top;
        int mainPadEnd    = isRow ? (int)m_padding.right  : (int)m_padding.bottom;
        int crossPadStart = isRow ? (int)m_padding.top    : (int)m_padding.left;
        int availMain  = (isRow ? bounds.w : bounds.h) - mainPadStart - mainPadEnd;
        m_lastAppliedMainSize = availMain;
        int availCross = (isRow ? bounds.h : bounds.w)
                         - crossPadStart
                         - (isRow ? (int)m_padding.bottom : (int)m_padding.right);

        struct ItemInfo {
            Element* elem;
            int mainSize;
            int crossSize;
            float grow;
        };

        std::vector<ItemInfo> allItems;
        for (auto* item : m_items) {
            if (!item->IsVisible())
                continue;
            Size s = item->GetPreferredSize();
            allItems.push_back({item, isRow ? s.w : s.h, isRow ? s.h : s.w, item->GetFlexGrow()});
        }

        if (allItems.empty())
            return;

        struct Line {
            std::vector<ItemInfo> items;
            int crossSize{0};
        };

        std::vector<Line> lines;

        if (m_wrap == FlexWrap::NoWrap) {
            Line line;
            for (auto& info : allItems)
                line.items.push_back(info);
            // Cross size is the full available cross for alignment purposes
            line.crossSize = availCross;
            lines.push_back(std::move(line));
        } else {
            Line currentLine;
            int currentMain = 0;

            for (auto& info : allItems) {
                int needed = currentLine.items.empty()
                    ? info.mainSize
                    : currentMain + (int)m_gap + info.mainSize;

                if (needed > availMain && !currentLine.items.empty()) {
                    lines.push_back(std::move(currentLine));
                    currentLine = Line{};
                    currentMain = info.mainSize;
                } else {
                    currentMain = needed;
                }

                currentLine.crossSize = std::max(currentLine.crossSize, info.crossSize);
                currentLine.items.push_back(info);
            }

            if (!currentLine.items.empty())
                lines.push_back(std::move(currentLine));

            if (m_wrap == FlexWrap::WrapReverse)
                std::reverse(lines.begin(), lines.end());
        }

        int crossPos = crossPadStart;

        for (auto& line : lines) {
            int n = (int)line.items.size();
            int lineCross = line.crossSize;

            // Distribute flexGrow within the line
            float totalGrow = 0.0f;
            int totalFixedMain = 0;
            for (auto& info : line.items) {
                if (info.grow <= 0)
                    totalFixedMain += info.mainSize;
                totalGrow += info.grow;
            }
            int totalGaps = (n - 1) * (int)m_gap;
            int spaceForGrow = availMain - totalFixedMain - totalGaps;
            if (totalGrow > 0 && spaceForGrow > 0) {
                for (auto& info : line.items) {
                    if (info.grow > 0)
                        info.mainSize = (int)(spaceForGrow * info.grow / totalGrow);
                }
            }

            int totalMain = 0;
            for (auto& info : line.items)
                totalMain += info.mainSize;
            int freeSpace = std::max(0, availMain - totalMain - totalGaps);

            float mainPos = static_cast<float>(mainPadStart);
            float itemGap = static_cast<float>(m_gap);

            switch (m_justify) {
                case FlexJustify::Start:
                    break;
                case FlexJustify::End:
                    mainPos += (float)freeSpace;
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
                    mainPos = (float)mainPadStart + space;
                    itemGap = space;
                    break;
                }
            }

            for (auto& info : line.items) {
                int itemCrossPos = crossPos;
                int itemCrossSize = info.crossSize;

                switch (m_align) {
                    case FlexAlign::Start:
                        break;
                    case FlexAlign::Center:
                        itemCrossPos += (lineCross - itemCrossSize) / 2;
                        break;
                    case FlexAlign::End:
                        itemCrossPos += lineCross - itemCrossSize;
                        break;
                    case FlexAlign::Stretch:
                        itemCrossSize = lineCross;
                        break;
                }

                Rectangle newBounds{};
                if (isRow)
                    newBounds = Rectangle((int)mainPos, itemCrossPos, info.mainSize, itemCrossSize);
                else
                    newBounds = Rectangle(itemCrossPos, (int)mainPos, itemCrossSize, info.mainSize);
                info.elem->SetLocalBounds(newBounds);

                mainPos += info.mainSize + itemGap;
            }

            crossPos += lineCross + (int)m_gap;
        }
    }

    Size FlexLayout::GetLaidOutSize() const {
        bool isRow = (m_direction == FlexDirection::Row);
        int mainPad  = isRow ? m_padding.GetHorizontal() : m_padding.GetVertical();
        int crossPad = isRow ? m_padding.GetVertical()   : m_padding.GetHorizontal();

        if (m_wrap != FlexWrap::NoWrap && m_lastAppliedMainSize > 0) {
            struct LineInfo { int mainTotal{0}; int crossMax{0}; int count{0}; };
            std::vector<LineInfo> lines;
            LineInfo cur;

            for (auto* e : m_items) {
                if (!e || !e->IsVisible())
                    continue;
                Size s = e->GetPreferredSize();
                int ms = isRow ? s.w : s.h;
                int cs = isRow ? s.h : s.w;

                int needed = cur.count == 0 ? ms : cur.mainTotal + (int)m_gap + ms;
                if (needed > m_lastAppliedMainSize && cur.count > 0) {
                    lines.push_back(cur);
                    cur = LineInfo{};
                    cur.mainTotal = ms;
                } else {
                    cur.mainTotal = needed;
                }
                cur.crossMax = std::max(cur.crossMax, cs);
                cur.count++;
            }
            if (cur.count > 0)
                lines.push_back(cur);

            int crossTotal = 0;
            int mainMax = 0;
            for (auto& line : lines) {
                crossTotal += line.crossMax;
                mainMax = std::max(mainMax, line.mainTotal);
            }
            if (lines.size() > 1)
                crossTotal += (int)(lines.size() - 1) * (int)m_gap;

            crossTotal += crossPad;
            mainMax += mainPad;

            if (isRow)
                return {mainMax, crossTotal};
            else
                return {crossTotal, mainMax};
        }

        int mainTotal = 0;
        int crossMax = 0;
        int visibleCount = 0;

        for (auto* e : m_items) {
            if (!e || !e->IsVisible())
                continue;
            Size s = e->GetPreferredSize();
            int ms = isRow ? s.w : s.h;
            int cs = isRow ? s.h : s.w;
            mainTotal += ms;
            if (cs > crossMax)
                crossMax = cs;
            visibleCount++;
        }

        if (visibleCount > 1)
            mainTotal += (visibleCount - 1) * (int)m_gap;
        mainTotal += mainPad;
        crossMax += crossPad;

        if (isRow)
            return {mainTotal, crossMax};
        else
            return {crossMax, mainTotal};
    }
} // namespace gui
