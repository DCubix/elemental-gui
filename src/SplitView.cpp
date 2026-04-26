#include "SplitView.h"

#include <stdexcept>

constexpr int MIN_SPLIT = 10;
constexpr int GRIPPER_SIZE = 6;

namespace gui {
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
        g.RoundRect(grip.x, grip.y, grip.w, grip.h, 2);
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
        Size size = GetSize();
        int splitPos = std::clamp(m_splitPosition, MIN_SPLIT, size.h - MIN_SPLIT);
        if (Element* first = GetFirst()) {
            first->SetPosition({ 0, 0 });
            first->SetSize({ size.w, splitPos - halfGrip });
        }
        if (Element* second = GetSecond()) {
            second->SetPosition({ 0, splitPos + halfGrip });
            second->SetSize({ size.w, size.h - splitPos - halfGrip });
        }
    }

    void SplitView::ResolveVertical()
    {
        const int halfGrip = GRIPPER_SIZE / 2;
        Size size = GetSize();
        int splitPos = std::clamp(m_splitPosition, MIN_SPLIT, size.w - MIN_SPLIT);
        if (Element* first = GetFirst()) {
            first->SetPosition({ 0, 0 });
            first->SetSize({ splitPos - halfGrip, size.h });
        }
        if (Element* second = GetSecond()) {
            second->SetPosition({ splitPos + halfGrip, 0 });
            second->SetSize({ size.w - splitPos - halfGrip, size.h });
        }
    }

    void SplitView::HandleDragHorizontal(int mouseY)
    {
        Size size = GetSize();
        int newSplitPos = std::clamp(mouseY, MIN_SPLIT, size.h - MIN_SPLIT);
        if (newSplitPos != m_splitPosition) {
            m_splitPosition = newSplitPos;
            Invalidate();
        }
    }

    void SplitView::HandleDragVertical(int mouseX)
    {
        Size size = GetSize();
        int newSplitPos = std::clamp(mouseX, MIN_SPLIT, size.w - MIN_SPLIT);
        if (newSplitPos != m_splitPosition) {
            m_splitPosition = newSplitPos;
            Invalidate();
        }
    }

    Rectangle SplitView::GripRectHorizontal() const
    {
        Size size = GetSize();
        int splitPos = std::clamp(m_splitPosition, MIN_SPLIT, size.h - MIN_SPLIT);
        return Rectangle(
            0,
            splitPos - GRIPPER_SIZE / 2,
            size.w,
            GRIPPER_SIZE
        );
    }

    Rectangle SplitView::GripRectVertical() const
    {
        Size size = GetSize();
        int splitPos = std::clamp(m_splitPosition, MIN_SPLIT, size.w - MIN_SPLIT);
        return Rectangle(
            splitPos - GRIPPER_SIZE / 2,
            0,
            GRIPPER_SIZE,
            size.h
        );
    }
    
    Rectangle SplitView::GripRect() const
    {
        return m_direction == Direction::Horizontal
            ? GripRectHorizontal() : GripRectVertical();
    }
}