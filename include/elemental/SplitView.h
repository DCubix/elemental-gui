#pragma once

#include "Container.h"

namespace gui {
    enum class SplitViewAlign { Start = 0, End };

    class SplitView : public Container {
    public:
        SplitView();

        void Add(Element* element) override;

        void OnDraw(Graphics& g) override;
        EventStatus OnEvent(Event* event) override;

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseMove(MotionEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;

        Size GetPreferredSize() const override;

        SplitViewAlign GetAlign() const { return m_align; }
        void SetAlign(SplitViewAlign align) {
            m_align = align;
            Invalidate();
        }

        Property<Direction> direction{Direction::Horizontal};
        Property<int> splitPosition{100};

    private:
        enum class ButtonState { Normal = 0, Hover, Click };

        SplitViewAlign m_align{SplitViewAlign::Start};

        ButtonState m_state{ButtonState::Normal};

        Element* GetFirst() { return m_children.size() > 0 ? m_children[0] : nullptr; }
        Element* GetSecond() { return m_children.size() > 1 ? m_children[1] : nullptr; }

        void ResolveHorizontal();
        void ResolveVertical();

        void HandleDragHorizontal(int mouseY);
        void HandleDragVertical(int mouseX);

        Rectangle GripRectHorizontal() const;
        Rectangle GripRectVertical() const;
        Rectangle GripRect() const;
    };
} // namespace gui