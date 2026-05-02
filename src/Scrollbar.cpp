#include "Scrollbar.h"

#include "Application.h"

namespace gui {

    Scrollbar::Scrollbar()
        : Element(),
          m_range(Range(0, 100)),
          m_state(ButtonState::Normal),
          m_dragOffset(0) {
        SetLocalBounds(Rectangle(0, 0, 120, 22));
        value.SetTransformer([this](const float& v) {
            float constrained = m_range.Constrain(v);
            return std::floor(constrained / step()) * step();
        });
        direction.SetOnUpdate([this] { Invalidate(); });
        value.SetOnUpdate([this] { Invalidate(); });
        step.SetOnUpdate([this] { Invalidate(); });
    }

    void Scrollbar::OnDraw(Graphics& g) {
        Size size = GetSize();
        int btn = GetButtonSize();
        int axisLen = (direction() == Direction::Horizontal ? size.w : size.h) - btn;
        m_buttonPos = int(m_range.Normalized(value()) * float(axisLen));

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
        if (direction() == Direction::Horizontal) {
            g.StyledRect(m_buttonPos, 0, btn, size.h, thumbStyle);
        } else {
            g.StyledRect(0, m_buttonPos, size.w, btn, thumbStyle);
        }
    }

    void Scrollbar::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        int p = (direction() == Direction::Horizontal ? e.x : e.y);
        int btn = GetButtonSize();

        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Click;
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
        float delta = (direction() == Direction::Horizontal ? e.scrollY : -e.scrollY);
        value = m_range.Constrain(value() + delta * step());
    }

    void Scrollbar::OnMouseMove(MotionEvent e) {
        if (m_state == ButtonState::Click) {
            int p = (direction() == Direction::Horizontal ? e.x : e.y);
            UpdateValue(p - m_dragOffset);
        }
    }

    void Scrollbar::SetRange(float min, float max) {
        m_range.minimum = min;
        m_range.maximum = max;
        value = m_range.Constrain(value());
        Invalidate();
    }

    void Scrollbar::UpdateValue(int p) {
        int btn = GetButtonSize();
        Size size = GetSize();
        p -= btn / 2;
        int axisLen = (direction() == Direction::Horizontal ? size.w : size.h) - btn;

        Range vp{0, float(axisLen)};
        value = m_range.Remap(vp, p);
    }

    int Scrollbar::GetButtonSize() {
        Size size = GetSize();
        int vpSize = (direction() == Direction::Horizontal ? size.w : size.h);
        float contentSize = (m_range.maximum - m_range.minimum) + vpSize;
        float viewRatio = vpSize / contentSize;
        int btnSize = int(vpSize * viewRatio);
        return btnSize < ScrollbarButtonSize ? ScrollbarButtonSize : btnSize;
    }

} // namespace gui
