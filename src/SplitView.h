#pragma once

#include "Container.h"

namespace gui {
    class SplitView : public Container {
    public:
        SplitView();

        void Add(Element *element) override;

        void OnDraw(Graphics& g) override;

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseMove(MotionEvent e) override;

        Size GetPreferredSize() const override;

        Direction GetDirection() const { return m_direction; }
        void SetDirection(Direction dir) { m_direction = dir; Invalidate(); }

        int GetSplitPosition() const { return m_splitPosition; }
        void SetSplitPosition(int pos) { m_splitPosition = pos; Invalidate(); }

    private:
        Direction m_direction{ Direction::Horizontal };
        int m_splitPosition{ 100 }; // px
        bool m_dragging{ false };

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
}