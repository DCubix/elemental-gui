#pragma once

#include "Element.h"
#include "Scrollbar.h"

namespace gui {
    class ScrollView : public Element {
      public:
        ScrollView();

        std::string StyleKey() const override { return "Panel"; }

        void OnScroll(ScrollEvent e) override;
        void OnDraw(Graphics& g) override;

        EventStatus OnEvent(Event* event) override;

        void SetElement(Element* e);
        Element* GetElement() { return m_element; }

        Direction GetScrollDirection() const { return m_scrollDirection; }
        void SetScrollDirection(Direction dir);

      private:
        Direction m_scrollDirection;
        Scrollbar* m_scrollBar;
        Element* m_element;

        bool IsDirty() override;

        void SolveScrollVertical();
        void SolveScrollHorizontal();
    };
} // namespace gui
