#include "Scrollbar.h"

#include "Application.h"

namespace gui {

    Scrollbar::Scrollbar()
        : Element(),
          m_direction(Direction::Horizontal),
          m_range(Range(0, 100)),
          m_value(0),
          m_step(1),
          m_state(ButtonState::Normal),
          m_dragOffset(0) {
        SetLocalBounds(Rectangle(0, 0, 120, 22));
    }

    void Scrollbar::OnDraw(Graphics& g) {
        Size size = GetSize();
        int btn = GetButtonSize();
        int axisLen = (m_direction == Direction::Horizontal ? size.w : size.h) - btn;
        m_buttonPos = int(m_range.Normalized(m_value) * float(axisLen));

        g.StyledRect(0, 0, size.w, size.h, GetStyle()["track"]);

        std::string state = "normal";
        switch (m_state) {
            case ButtonState::Normal:
                state = "normal";
                break;
            case ButtonState::Hover:
                state = "hover";
                break;
            case ButtonState::Click:
                state = "click";
                break;
        }

        Json thumbStyle = GetStyle()["thumb"][state];
        if (m_direction == Direction::Horizontal) {
            g.StyledRect(m_buttonPos, 0, btn, size.h, thumbStyle);
        } else {
            g.StyledRect(0, m_buttonPos, size.w, btn, thumbStyle);
        }
    }

    void Scrollbar::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        Rectangle c = GetLocalBounds();
        int p = (m_direction == Direction::Horizontal ? e.x : e.y);
        int btn = GetButtonSize();

        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Click;
            // If clicking on the thumb, track offset; otherwise center thumb on click
            if (p >= m_buttonPos && p < m_buttonPos + btn) {
                m_dragOffset = p - m_buttonPos - btn / 2;
            } else {
                m_dragOffset = 0;
                UpdateValue(p);
            }
            Invalidate();
        }
    }

    void Scrollbar::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (m_state == ButtonState::Click) {
            Rectangle b = GetLocalBounds();
            m_state = b.HasPoint(e.x, e.y) ? ButtonState::Hover : ButtonState::Normal;
            Invalidate();
        }
    }

    void Scrollbar::OnMouseEnter() {
        if (m_state == ButtonState::Normal) {
            m_state = ButtonState::Hover;
            Invalidate();
        }
    }

    void Scrollbar::OnMouseLeave() {
        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Normal;
            Invalidate();
        }
    }

    void Scrollbar::OnScroll(ScrollEvent e) {
        float delta = (m_direction == Direction::Horizontal ? e.scrollY : -e.scrollY);
        SetValue(m_range.Constrain(m_value + delta * m_step));
    }

    void Scrollbar::OnMouseMove(MotionEvent e) {
        // Only handle dragging during Click state - hover is handled by OnMouseEnter/OnMouseLeave
        if (m_state == ButtonState::Click) {
            int p = (m_direction == Direction::Horizontal ? e.x : e.y);
            UpdateValue(p - m_dragOffset);
        }
    }

    void Scrollbar::SetRange(float min, float max) {
        m_range.minimum = min;
        m_range.maximum = max;
        SetValue(m_range.Constrain(m_value));
        Invalidate();
    }

    void Scrollbar::SetValue(float v) {
        if (m_onValueChange && m_value != v)
            m_onValueChange(m_value);
        m_value = v;
        Invalidate();
    }

    void Scrollbar::UpdateValue(int p) {
        int btn = GetButtonSize();
        Size size = GetSize();
        p -= btn / 2;
        int axisLen = (m_direction == Direction::Horizontal ? size.w : size.h) - btn;

        Range vp{0, float(axisLen)};
        m_value = m_range.Constrain(m_range.Remap(vp, p));
        SetValue(std::floor(m_value / m_step) * m_step);
        Invalidate();
    }

    int Scrollbar::GetButtonSize() {
        Size size = GetSize();
        int vpSize = (m_direction == Direction::Horizontal ? size.w : size.h);
        float contentSize = (m_range.maximum - m_range.minimum) + vpSize;
        float viewRatio = vpSize / contentSize;
        int btnSize = int(vpSize * viewRatio);
        return btnSize < ScrollbarButtonSize ? ScrollbarButtonSize : btnSize;
    }

} // namespace gui
