#include "Slider.h"

#include "Application.h"

#include <cmath>

namespace gui {

    Slider::Slider()
        : Element(),
          m_range(Range(0, 100)),
          m_state(ButtonState::Normal),
          m_thumbPos(0),
          m_dragOffset(0) {
        SetLocalBounds(Rectangle(0, 0, 120, 22));
        value.SetTransformer([this](const float& v) {
            float constrained = m_range.Constrain(v);
            return std::floor(constrained / step()) * step();
        });
        TrackAll(direction, value, step);
    }

    void Slider::OnDraw(Graphics& g) {
        auto sz = GetSize();
        int thumb = SliderThumbSize;
        int trackLen = (direction() == Direction::Horizontal ? sz.w : sz.h) - thumb;
        m_thumbPos = int(m_range.Normalized(value()) * float(trackLen));

        // reverse when vertical to have min at bottom and max at top
        if (direction() == Direction::Vertical)
            m_thumbPos = trackLen - m_thumbPos;

        // Draw thin track centered in the element
        Json trackStyle = GetStyle()["track"];
        int trackSize = trackStyle.value("size", 8);
        if (direction() == Direction::Horizontal) {
            int trackH = trackSize;
            int trackY = (sz.h - trackH) / 2;
            g.StyledRect(0, trackY, sz.w, trackH, trackStyle);
        } else {
            int trackW = trackSize;
            int trackX = (sz.w - trackW) / 2;
            g.StyledRect(trackX, 0, trackW, sz.h, trackStyle);
        }

        // Draw fixed-size thumb
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
            g.StyledRect(m_thumbPos, 0, thumb, sz.h, thumbStyle);
        } else {
            g.StyledRect(0, m_thumbPos, sz.w, thumb, thumbStyle);
        }
    }

    void Slider::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        int p = (direction() == Direction::Horizontal ? e.x : e.y);
        int thumb = SliderThumbSize;

        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Click;
            if (p >= m_thumbPos && p < m_thumbPos + thumb) {
                m_dragOffset = p - m_thumbPos - thumb / 2;
            } else {
                m_dragOffset = 0;
                UpdateValue(p);
            }
            Invalidate();
        }
    }

    void Slider::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
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
        float delta = (direction() == Direction::Horizontal ? e.scrollY : -e.scrollY);
        value = m_range.Constrain(value() + delta * step());
    }

    void Slider::OnMouseMove(MotionEvent e) {
        if (m_state == ButtonState::Click) {
            int p = (direction() == Direction::Horizontal ? e.x : e.y);
            UpdateValue(p - m_dragOffset);
        }
    }

    void Slider::SetRange(float min, float max) {
        m_range.minimum = min;
        m_range.maximum = max;
        value = m_range.Constrain(value());
        Invalidate();
    }

    void Slider::UpdateValue(int p) {
        int thumb = SliderThumbSize;
        Size size = GetSize();
        p -= thumb / 2;
        int axisLen = (direction() == Direction::Horizontal ? size.w : size.h) - thumb;

        // reverse when vertical to have min at bottom and max at top
        if (direction() == Direction::Vertical)
            p = axisLen - p;

        Range vp{0, float(axisLen)};
        value = m_range.Remap(vp, p);
    }

} // namespace gui
