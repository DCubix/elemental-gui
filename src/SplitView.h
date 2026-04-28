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

        Direction GetDirection() const { return m_direction; }
        void SetDirection(Direction dir) {
            m_direction = dir;
            Invalidate();
        }

        SplitViewAlign GetAlign() const { return m_align; }
        void SetAlign(SplitViewAlign align) {
            m_align = align;
            Invalidate();
        }

        int GetSplitPosition() const { return m_splitPosition; }
        void SetSplitPosition(int pos) {
            m_splitPosition = pos;
            Invalidate();
        }

    private:
        enum class ButtonState { Normal = 0, Hover, Click };

        Direction m_direction{Direction::Horizontal};
        SplitViewAlign m_align{SplitViewAlign::Start};
        int m_splitPosition{100}; // px

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