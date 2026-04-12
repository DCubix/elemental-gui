#include "Button.h"

#include "Application.h"

namespace tui {

	Button::Button()
		: Label()
	{
		GetLocalBounds().h = 22;
		m_alignment = Alignment::MiddleCenter;
		m_text = "Button";
		m_state = ButtonState::Normal;
	}

	void Button::OnDraw(Graphics& g) {
		Rectangle b = GetBounds();

		std::string state = "normal";
		switch (m_state) {
			case ButtonState::Normal: state = "normal"; break;
			case ButtonState::Hover: state = "hover"; break;
			case ButtonState::Click: state = "click"; break;
		}

		g.StyledRect(b.x, b.y, b.w, b.h, GetApp()->GetStyle()["Button"][state]);
		Label::OnDraw(g);
	}

	EventStatus Button::OnEvent(Event* event) {
		EventStatus status = Element::OnEvent(event);
		if (event->Type() == EventType::MouseEventType) {
			Rectangle b = GetIntersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			switch (m_state) {
				case ButtonState::Hover: {
					if (e->pressed && e->button == 1) {
						m_state = ButtonState::Click;
						status = EventStatus::Consumed;
						Invalidate();
					}
				} break;
				case ButtonState::Click: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Normal;
						status = EventStatus::Consumed;
						Invalidate();
					}
					if (!e->pressed && e->button == 1) {
						if (m_onClick)
							m_onClick();
						m_state = ButtonState::Hover;
						status = EventStatus::Consumed;
						Invalidate();
					}
				} break;
				default: break;
			}
		} else if (event->Type() == EventType::MotionEventType) {
			Rectangle b = GetIntersectedBounds();
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			switch (m_state) {
				case ButtonState::Normal: {
					if (b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Hover;
						Invalidate();
						status = EventStatus::Consumed;
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
					if (!b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Normal;
						Invalidate();
					} else {
						status = EventStatus::Consumed;
					}
				} break;
			}
		}
		return status;
	}

}
