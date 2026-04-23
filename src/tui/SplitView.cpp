#include "SplitView.h"

#include <stdexcept>

constexpr int MIN_SPLIT = 10;
constexpr int GRIPPER_SIZE = 6;

namespace tui {
    SplitView::SplitView() : Container()
    {
        SetLocalBounds(Rectangle(0, 0, 300, 300));
    }

    void SplitView::Add(Element *element)
    {
        if (m_children.size() >= 2) {
            throw std::runtime_error("SplitView can only contain 2 children");
        }
        Container::Add(element);
    }

    void SplitView::OnDraw(Graphics &g)
    {
        if (m_direction == Direction::Horizontal) {
            ResolveHorizontal();
        } else {
            ResolveVertical();
        }

        Container::OnDraw(g);

        Rectangle grip = GripRect();
        
        g.Color(0.5f, 0.5f, 0.5f, 0.5f);
        if (m_direction == Direction::Horizontal) {
            g.RoundRect(grip.x, grip.y, grip.w, grip.h, 2);
        } else {
            g.RoundRect(grip.x, grip.y, grip.w, grip.h, 2);
        }
        g.Fill();
    }

    void SplitView::OnMouseDown(MouseEvent e) {
		if (e.button != MouseButton::Left) return;
		if (GripRect().HasPoint(e.x, e.y)) {
			m_dragging = true;
		}
    }

	void SplitView::OnMouseUp(MouseEvent e) {
		if (e.button != MouseButton::Left) return;
		m_dragging = false;
	}

	void SplitView::OnMouseMove(MotionEvent e) {
		if (!m_dragging) return;
		if (m_direction == Direction::Horizontal) {
			HandleDragHorizontal(e.y);
		} else {
			HandleDragVertical(e.x);
		}
	}

    Size SplitView::GetPreferredSize() const
    {
        Size firstSize{0, 0}, secondSize{0, 0};
        if (Element* first = m_children.size() > 0 ? m_children[0] : nullptr) {
            firstSize = first->GetPreferredSize();
        }
        if (Element* second = m_children.size() > 1 ? m_children[1] : nullptr) {
            secondSize = second->GetPreferredSize();
        }

        if (m_direction == Direction::Horizontal) {
            return { firstSize.w + secondSize.w + GRIPPER_SIZE, std::max(firstSize.h, secondSize.h) };
        } else {
            return { std::max(firstSize.w, secondSize.w), firstSize.h + secondSize.h + GRIPPER_SIZE };
        }
    }

    void SplitView::ResolveHorizontal()
    {
        const int halfGrip = GRIPPER_SIZE / 2;
        Rectangle b = GetBounds();
        int splitPos = std::clamp(m_splitPosition, MIN_SPLIT, b.h - MIN_SPLIT);
        if (Element* first = GetFirst()) {
            first->SetPosition({ b.x, b.y });
            first->SetSize({ b.w, splitPos - halfGrip });
        }
        if (Element* second = GetSecond()) {
            second->SetPosition({ b.x, b.y + splitPos + halfGrip });
            second->SetSize({ b.w, b.h - splitPos - halfGrip });
        }
    }
    
    void SplitView::ResolveVertical()
    {
        const int halfGrip = GRIPPER_SIZE / 2;
        Rectangle b = GetBounds();
        int splitPos = std::clamp(m_splitPosition, MIN_SPLIT, b.w - MIN_SPLIT);
        if (Element* first = GetFirst()) {
            first->SetPosition({ b.x, b.y });
            first->SetSize({ splitPos - halfGrip, b.h });
        }
        if (Element* second = GetSecond()) {
            second->SetPosition({ b.x + splitPos + halfGrip, b.y });
            second->SetSize({ b.w - splitPos - halfGrip, b.h });
        }
    }
    
    void SplitView::HandleDragHorizontal(int mouseY)
    {
        Rectangle b = GetBounds();
        int newSplitPos = std::clamp(mouseY - b.y, MIN_SPLIT, b.h - MIN_SPLIT);
        if (newSplitPos != m_splitPosition) {
            m_splitPosition = newSplitPos;
            Invalidate();
        }
    }

    void SplitView::HandleDragVertical(int mouseX)
    {
        Rectangle b = GetBounds();
        int newSplitPos = std::clamp(mouseX - b.x, MIN_SPLIT, b.w - MIN_SPLIT);
        if (newSplitPos != m_splitPosition) {
            m_splitPosition = newSplitPos;
            Invalidate();
        }
    }

    Rectangle SplitView::GripRectHorizontal() const
    {
        Rectangle b = GetBounds();
        int splitPos = std::clamp(m_splitPosition, MIN_SPLIT, b.h - MIN_SPLIT);
        return Rectangle(
            b.x,
            b.y + splitPos - GRIPPER_SIZE / 2,
            b.w,
            GRIPPER_SIZE
        );
    }
    
    Rectangle SplitView::GripRectVertical() const
    {
        Rectangle b = GetBounds();
        int splitPos = std::clamp(m_splitPosition, MIN_SPLIT, b.w - MIN_SPLIT);
        return Rectangle(
            b.x + splitPos - GRIPPER_SIZE / 2,
            b.y,
            GRIPPER_SIZE,
            b.h
        );
    }
    
    Rectangle SplitView::GripRect() const
    {
        return m_direction == Direction::Horizontal
            ? GripRectHorizontal() : GripRectVertical();
    }
}