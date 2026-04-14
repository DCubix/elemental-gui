#include "Scrollbar.h"

#include "Application.h"

#include <iostream>

namespace tui {

	Scrollbar::Scrollbar()
		: Element(),
		  m_direction(Direction::Horizontal),
		  m_range(Range(0, 100)),
		  m_value(0),
		  m_step(1),
		  m_state(ButtonState::Normal),
		  m_dragOffset(0)
	{
		SetLocalBounds(Rectangle(0, 0, 120, 22));
	}

	void Scrollbar::OnDraw(Graphics& g) {
		Rectangle b = GetBounds();
		int btn = GetButtonSize();
		int sz = (m_direction == Direction::Horizontal ? b.w : b.h) - btn;
		m_buttonPos = int(m_range.Normalized(m_value) * float(sz));

		g.StyledRect(b.x, b.y, b.w, b.h, GetStyle()["Scrollbar"]["track"]);

		std::string state = "normal";
		switch (m_state) {
			case ButtonState::Normal: state = "normal"; break;
			case ButtonState::Hover: state = "hover"; break;
			case ButtonState::Click: state = "click"; break;
		}

		Json thumbStyle = GetStyle()["Scrollbar"]["thumb"][state];
		if (m_direction == Direction::Horizontal) {
			g.StyledRect(b.x + m_buttonPos, b.y, btn, b.h, thumbStyle);
		} else {
			g.StyledRect(b.x, b.y + m_buttonPos, b.w, btn, thumbStyle);
		}
	}

	EventStatus Scrollbar::OnEvent(Event* event) {
		EventStatus status = Element::OnEvent(event);
		if (event->Type() == EventType::MouseButton) {
			Rectangle b = GetIntersectedBounds();
			Rectangle c = GetBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			int p = (m_direction == Direction::Horizontal ? e->x - c.x : e->y - c.y);
			int btn = GetButtonSize();
			switch (m_state) {
				case ButtonState::Hover: {
					if (e->pressed && e->button == 1) {
						m_state = ButtonState::Click;
						// If clicking on the thumb, track offset; otherwise center thumb on click
						if (p >= m_buttonPos && p < m_buttonPos + btn) {
							m_dragOffset = p - m_buttonPos - btn / 2;
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

	void Scrollbar::SetRange(float min, float max) {
		m_range.minimum = min;
		m_range.maximum = max;
		SetValue(m_range.Constrain(m_value));
		Invalidate();
	}

	void Scrollbar::SetValue(float v) {
		if (m_onValueChange && m_value != v) m_onValueChange(m_value);
		m_value = v; Invalidate();
	}

	void Scrollbar::UpdateValue(int p) {
		int btn = GetButtonSize();
		Rectangle b = GetBounds();
		p -= btn/2;
		int sz = (m_direction == Direction::Horizontal ? b.w : b.h) - btn;

		Range vp{ 0, float(sz) };
		m_value = m_range.Constrain(m_range.Remap(vp, p));
		SetValue(std::floor(m_value / m_step) * m_step);
		Invalidate();
	}

	int Scrollbar::GetButtonSize() {
		Rectangle b = GetBounds();
		int vpSize = (m_direction == Direction::Horizontal ? b.w : b.h);
		float contentSize = (m_range.maximum - m_range.minimum) + vpSize;
		float viewRatio = vpSize / contentSize;
		int btnSize = int(vpSize * viewRatio);
		return btnSize < ScrollbarButtonSize ? ScrollbarButtonSize : btnSize;
	}

}
