#pragma once

#include "Layout.h"

#include <vector>

namespace gui {
    class Element;

    enum class FlexDirection { Row, Column };
    enum class FlexJustify { Start, Center, End, SpaceBetween, SpaceEvenly };
    enum class FlexAlign { Start, Center, End, Stretch };
    enum class FlexWrap { NoWrap, Wrap, WrapReverse };

    class FlexLayout : public Layout {
    public:
        FlexLayout(
            FlexDirection dir = FlexDirection::Row,
            FlexJustify justify = FlexJustify::Start,
            FlexAlign align = FlexAlign::Stretch
        );

        void Add(Element* element);
        void Remove(Element* element);
        void Apply(const Rectangle& bounds) override;
        Size GetLaidOutSize() const override;

        void SetDirection(FlexDirection dir) { m_direction = dir; }
        FlexDirection GetDirection() const { return m_direction; }

        void SetJustifyContent(FlexJustify justify) { m_justify = justify; }
        FlexJustify GetJustifyContent() const { return m_justify; }

        void SetAlignItems(FlexAlign align) { m_align = align; }
        FlexAlign GetAlignItems() const { return m_align; }

        void SetWrap(FlexWrap wrap) { m_wrap = wrap; }
        FlexWrap GetWrap() const { return m_wrap; }

    private:
        FlexDirection m_direction;
        FlexJustify m_justify;
        FlexAlign m_align;
        FlexWrap m_wrap{FlexWrap::NoWrap};
        std::vector<Element*> m_items;
        int m_lastAppliedMainSize{0};
    };
} // namespace gui
