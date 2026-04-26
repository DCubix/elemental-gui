#include "Slider.h"

#include "Application.h"

#include <cmath>

namespace gui
{

    Slider::Slider()
        : Element(),
        m_direction(Direction::Horizontal),
        m_range(Range(0, 100)),
        m_value(0),
        m_step(1),
        m_state(ButtonState::Normal),
        m_thumbPos(0),
        m_dragOffset(0) {
        SetLocalBounds(Rectangle(0, 0, 120, 22));
    }

    void Slider::OnDraw(Graphics& g) {
        auto sz = GetSize();
        int thumb = SliderThumbSize;
        int trackLen = (m_direction == Direction::Horizontal ? sz.w : sz.h) - thumb;
        m_thumbPos = int(m_range.Normalized(m_value) * float(trackLen));

        // Draw thin track centered in the element
        Json trackStyle = GetStyle()["track"];
        if (m_direction == Direction::Horizontal) {
            int trackH = std::max(4, sz.h / 3);
            int trackY = (sz.h - trackH) / 2;
            g.StyledRect(0, trackY, sz.w, trackH, trackStyle);
        }
        else {
            int trackW = std::max(4, sz.w / 3);
            int trackX = (sz.w - trackW) / 2;
            g.StyledRect(trackX, 0, trackW, sz.h, trackStyle);
        }

        // Draw fixed-size thumb
        std::string state = "normal";
        switch (m_state) {
            case ButtonState::Normal: state = "normal"; break;
            case ButtonState::Hover: state = "hover"; break;
            case ButtonState::Click: state = "click"; break;
        }

        Json thumbStyle = GetStyle()["thumb"][state];
        if (m_direction == Direction::Horizontal) {
            g.StyledRect(m_thumbPos, 0, thumb, sz.h, thumbStyle);
        }
        else {
            g.StyledRect(0, m_thumbPos, sz.w, thumb, thumbStyle);
        }
    }

    void Slider::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        int p = (m_direction == Direction::Horizontal ? e.x : e.y);
        int thumb = SliderThumbSize;

        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Click;
            if (p >= m_thumbPos && p < m_thumbPos + thumb) {
                m_dragOffset = p - m_thumbPos - thumb / 2;
            }
            else {
                m_dragOffset = 0;
                UpdateValue(p);
            }
            Invalidate();
        }
    }

    void Slider::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        if (m_state == ButtonState::Click) {
            Rectangle b = GetLocalBounds();
            m_state = b.HasPoint(e.x, e.y) ? ButtonState::Hover : ButtonState::Normal;
            Invalidate();
        }
    }

    void Slider::OnMouseEnter() {
        if (m_state == ButtonState::Normal) {
            m_state = ButtonState::Hover;
            Invalidate();
        }
    }

    void Slider::OnMouseLeave() {
        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Normal;
            Invalidate();
        }
    }

    void Slider::OnScroll(ScrollEvent e) {
        float delta = (m_direction == Direction::Horizontal ? e.scrollY : -e.scrollY);
        SetValue(m_range.Constrain(m_value + delta * m_step));
    }

    void Slider::OnMouseMove(MotionEvent e) {
        // Only handle dragging during Click state - hover is handled by OnMouseEnter/OnMouseLeave
        if (m_state == ButtonState::Click) {
            int p = (m_direction == Direction::Horizontal ? e.x : e.y);
            UpdateValue(p - m_dragOffset);
        }
    }

    void Slider::SetRange(float min, float max) {
        m_range.minimum = min;
        m_range.maximum = max;
        SetValue(m_range.Constrain(m_value));
        Invalidate();
    }

    void Slider::SetValue(float v) {
        if (m_onValueChange && m_value != v) m_onValueChange(m_value);
        m_value = v; Invalidate();
    }

    void Slider::UpdateValue(int p) {
        int thumb = SliderThumbSize;
        Size size = GetSize();
        p -= thumb / 2;
        int axisLen = (m_direction == Direction::Horizontal ? size.w : size.h) - thumb;

        Range vp{ 0, float(axisLen) };
        m_value = m_range.Constrain(m_range.Remap(vp, p));
        SetValue(std::floor(m_value / m_step) * m_step);
        Invalidate();
    }

}
