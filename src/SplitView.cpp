#include "SplitView.h"

#include <stdexcept>

constexpr int MIN_SPLIT = 32;
constexpr int GRIPPER_SIZE = 6;

namespace gui {
    SplitView::SplitView()
        : Container() {
        SetLocalBounds(Rectangle(0, 0, 300, 300));
        direction.SetOnUpdate([this]{ Invalidate(); });
        splitPosition.SetOnUpdate([this]{ Invalidate(); });
    }

    void SplitView::Add(Element* element) {
        if (m_children.size() >= 2) {
            throw std::runtime_error("SplitView can only contain 2 children");
        }
        Container::Add(element);
    }

    void SplitView::OnDraw(Graphics& g) {
        if (direction() == Direction::Horizontal) {
            ResolveHorizontal();
        } else {
            ResolveVertical();
        }

        Container::OnDraw(g);

        Rectangle grip = GripRect();
        g.Color(0.5f, 0.5f, 0.5f, 0.5f);
        if (direction() == Direction::Horizontal) {
            int x = (grip.x + grip.w / 2) - (GRIPPER_SIZE / 2);
            for (int i = 0; i < 3; i++) {
                int off = static_cast<int>(((float(i) / 2.0f) * 2.0f - 1.0f) * GRIPPER_SIZE * 2);
                g.RoundRect(
                    x + off,
                    grip.y + grip.h / 2 - GRIPPER_SIZE / 2,
                    GRIPPER_SIZE,
                    GRIPPER_SIZE,
                    999.0f
                );
                g.Fill();
            }
        } else {
            int y = (grip.y + grip.h / 2) - (GRIPPER_SIZE / 2);
            for (int i = 0; i < 3; i++) {
                int off = static_cast<int>(((float(i) / 2.0f) * 2.0f - 1.0f) * GRIPPER_SIZE * 2);
                g.RoundRect(
                    grip.x + grip.w / 2 - GRIPPER_SIZE / 2,
                    y + off,
                    GRIPPER_SIZE,
                    GRIPPER_SIZE,
                    999.0f
                );
                g.Fill();
            }
        }
    }

    EventStatus SplitView::OnEvent(Event* event) {
        // During active drag, bypass child dispatch so mouse events always reach
        // this element regardless of cursor position. Without this, a child panel
        // consumes mouse events whenever the cursor leaves the 6px grip zone,
        // breaking fast drags and causing stuck drag state on mouse-up inside a child.
        if (m_mouseDown &&
            (event->Type() == EventType::MouseMotion || event->Type() == EventType::MouseButton)) {
            return Element::OnEvent(event);
        }
        return Container::OnEvent(event);
    }

    void SplitView::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Click;
            Invalidate();
        }
    }

    void SplitView::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (m_state == ButtonState::Click) {
            Rectangle b = GetLocalBounds();
            m_state = b.HasPoint(e.x, e.y) ? ButtonState::Hover : ButtonState::Normal;
            Invalidate();
        }
    }

    void SplitView::OnMouseMove(MotionEvent e) {
        if (!m_mouseDown)
            return;

        if (direction() == Direction::Horizontal) {
            HandleDragHorizontal(e.y);
        } else {
            HandleDragVertical(e.x);
        }
        Invalidate();
    }

    void SplitView::OnMouseEnter() {
        if (m_state == ButtonState::Normal) {
            m_state = ButtonState::Hover;
            Invalidate();
        }
    }

    void SplitView::OnMouseLeave() {
        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Normal;
            Invalidate();
        }
    }

    Size SplitView::GetPreferredSize() const {
        Size firstSize{0, 0}, secondSize{0, 0};
        if (Element* first = m_children.size() > 0 ? m_children[0] : nullptr) {
            firstSize = first->GetPreferredSize();
        }
        if (Element* second = m_children.size() > 1 ? m_children[1] : nullptr) {
            secondSize = second->GetPreferredSize();
        }

        if (direction() == Direction::Horizontal) {
            return {firstSize.w + secondSize.w + GRIPPER_SIZE, std::max(firstSize.h, secondSize.h)};
        } else {
            return {std::max(firstSize.w, secondSize.w), firstSize.h + secondSize.h + GRIPPER_SIZE};
        }
    }

    void SplitView::ResolveHorizontal() {
        const int halfGrip = GRIPPER_SIZE / 2;
        Size size = GetSize();
        int splitPos = std::clamp(splitPosition(), MIN_SPLIT, size.h - MIN_SPLIT);
        if (Element* first = GetFirst()) {
            if (m_align == SplitViewAlign::Start) {
                first->SetPosition({0, 0});
                first->SetSize({size.w, splitPos - halfGrip});
            } else {
                first->SetPosition({0, 0});
                first->SetSize({size.w, size.h - splitPos - halfGrip});
            }
        }
        if (Element* second = GetSecond()) {
            if (m_align == SplitViewAlign::Start) {
                second->SetPosition({0, splitPos + halfGrip});
                second->SetSize({size.w, size.h - splitPos - halfGrip});
            } else {
                second->SetPosition({0, size.h - splitPos + halfGrip});
                second->SetSize({size.w, splitPos - halfGrip});
            }
        }
    }

    void SplitView::ResolveVertical() {
        const int halfGrip = GRIPPER_SIZE / 2;
        Size size = GetSize();
        int splitPos = std::clamp(splitPosition(), MIN_SPLIT, size.w - MIN_SPLIT);
        if (Element* first = GetFirst()) {
            if (m_align == SplitViewAlign::Start) {
                first->SetPosition({0, 0});
                first->SetSize({splitPos - halfGrip, size.h});
            } else {
                first->SetPosition({0, 0});
                first->SetSize({size.w - splitPos - halfGrip, size.h});
            }
        }
        if (Element* second = GetSecond()) {
            if (m_align == SplitViewAlign::Start) {
                second->SetPosition({splitPos + halfGrip, 0});
                second->SetSize({size.w - splitPos - halfGrip, size.h});
            } else {
                second->SetPosition({size.w - splitPos + halfGrip, 0});
                second->SetSize({splitPos - halfGrip, size.h});
            }
        }
    }

    void SplitView::HandleDragHorizontal(int mouseY) {
        Size size = GetSize();
        int newSplitPos;
        if (m_align == SplitViewAlign::Start) {
            newSplitPos = std::clamp(mouseY, MIN_SPLIT, size.h - MIN_SPLIT);
        } else {
            newSplitPos = std::clamp(size.h - mouseY, MIN_SPLIT, size.h - MIN_SPLIT);
        }
        splitPosition = newSplitPos;
    }

    void SplitView::HandleDragVertical(int mouseX) {
        Size size = GetSize();
        int newSplitPos;
        if (m_align == SplitViewAlign::Start) {
            newSplitPos = std::clamp(mouseX, MIN_SPLIT, size.w - MIN_SPLIT);
        } else {
            newSplitPos = std::clamp(size.w - mouseX, MIN_SPLIT, size.w - MIN_SPLIT);
        }
        splitPosition = newSplitPos;
    }

    Rectangle SplitView::GripRectHorizontal() const {
        Size size = GetSize();
        int splitPos = std::clamp(splitPosition(), MIN_SPLIT, size.h - MIN_SPLIT);
        int y = m_align == SplitViewAlign::Start ? splitPos : size.h - splitPos;
        return Rectangle(0, y - GRIPPER_SIZE / 2, size.w, GRIPPER_SIZE);
    }

    Rectangle SplitView::GripRectVertical() const {
        Size size = GetSize();
        int splitPos = std::clamp(splitPosition(), MIN_SPLIT, size.w - MIN_SPLIT);
        int x = m_align == SplitViewAlign::Start ? splitPos : size.w - splitPos;
        return Rectangle(x - GRIPPER_SIZE / 2, 0, GRIPPER_SIZE, size.h);
    }

    Rectangle SplitView::GripRect() const {
        return direction() == Direction::Horizontal ? GripRectHorizontal() : GripRectVertical();
    }
} // namespace gui