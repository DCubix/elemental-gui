#include "Slider.h"

#include "Application.h"

#include <cmath>

namespace tui {

	Slider::Slider()
		: Element(),
		  m_direction(Direction::Horizontal),
		  m_range(Range(0, 100)),
		  m_value(0),
		  m_step(1),
		  m_state(ButtonState::Normal),
		  m_thumbPos(0),
		  m_dragOffset(0)
	{
		SetLocalBounds(Rectangle(0, 0, 120, 22));
	}

	void Slider::OnDraw(Graphics& g) {
		Rectangle b = GetBounds();
		int thumb = SliderThumbSize;
		int trackLen = (m_direction == Direction::Horizontal ? b.w : b.h) - thumb;
		m_thumbPos = int(m_range.Normalized(m_value) * float(trackLen));

		// Draw thin track centered in the element
		Json trackStyle = GetStyle()["Slider"]["track"];
		if (m_direction == Direction::Horizontal) {
			int trackH = std::max(4, b.h / 3);
			int trackY = b.y + (b.h - trackH) / 2;
			g.StyledRect(b.x, trackY, b.w, trackH, trackStyle);
		} else {
			int trackW = std::max(4, b.w / 3);
			int trackX = b.x + (b.w - trackW) / 2;
			g.StyledRect(trackX, b.y, trackW, b.h, trackStyle);
		}

		// Draw fixed-size thumb
		std::string state = "normal";
		switch (m_state) {
			case ButtonState::Normal: state = "normal"; break;
			case ButtonState::Hover: state = "hover"; break;
			case ButtonState::Click: state = "click"; break;
		}

		Json thumbStyle = GetStyle()["Slider"]["thumb"][state];
		if (m_direction == Direction::Horizontal) {
			g.StyledRect(b.x + m_thumbPos, b.y, thumb, b.h, thumbStyle);
		} else {
			g.StyledRect(b.x, b.y + m_thumbPos, b.w, thumb, thumbStyle);
		}
	}

	EventStatus Slider::OnEvent(Event* event) {
		EventStatus status = Element::OnEvent(event);
		if (event->Type() == EventType::MouseButton) {
			Rectangle b = GetIntersectedBounds();
			Rectangle c = GetBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			int p = (m_direction == Direction::Horizontal ? e->x - c.x : e->y - c.y);
			int thumb = SliderThumbSize;
			switch (m_state) {
				case ButtonState::Hover: {
					if (e->pressed && e->button == 1) {
						m_state = ButtonState::Click;
						if (p >= m_thumbPos && p < m_thumbPos + thumb) {
							m_dragOffset = p - m_thumbPos - thumb / 2;
						} else {
							m_dragOffset = 0;
							UpdateValue(p);
						}
						status = EventStatus::Consumed;
						Invalidate();
					}
				} break;
				case ButtonState::Click: {
					if (!e->pressed && e->button == 1) {
						m_state = b.HasPoint(e->x, e->y) ? ButtonState::Hover : ButtonState::Normal;
						status = EventStatus::Consumed;
						Invalidate();
					} else {
						UpdateValue(p - m_dragOffset);
						status = EventStatus::Consumed;
					}
				} break;
				default: break;
			}
		} else if (event->Type() == EventType::MouseMotion) {
			Rectangle b = GetIntersectedBounds();
			Rectangle c = GetBounds();
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			int p = (m_direction == Direction::Horizontal ? e->x - c.x : e->y - c.y);
			switch (m_state) {
				case ButtonState::Normal: {
					if (b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Hover;
						status = EventStatus::Consumed;
						Invalidate();
					}
				} break;
				case ButtonState::Hover: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Normal;
						Invalidate();
					} else {
						status = EventStatus::Consumed;
					}
				} break;
				case ButtonState::Click: {
					UpdateValue(p - m_dragOffset);
					status = EventStatus::Consumed;
				} break;
			}
		} else if (event->Type() == EventType::Scroll) {
			Rectangle b = GetIntersectedBounds();
			ScrollEvent* e = dynamic_cast<ScrollEvent*>(event);
			if (b.HasPoint(e->mouseX, e->mouseY)) {
				float delta = (m_direction == Direction::Horizontal ? e->scrollY : -e->scrollY);
				SetValue(m_range.Constrain(m_value + delta * m_step));
				status = EventStatus::Consumed;
			}
		}
		return status;
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
		Rectangle b = GetBounds();
		p -= thumb / 2;
		int sz = (m_direction == Direction::Horizontal ? b.w : b.h) - thumb;

		Range vp{ 0, float(sz) };
		m_value = m_range.Constrain(m_range.Remap(vp, p));
		SetValue(std::floor(m_value / m_step) * m_step);
		Invalidate();
	}

}
