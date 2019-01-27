#include "Button.h"

#include "Application.h"

namespace tui {

	Button::Button()
		: Label()
	{
		localBounds().h = 22;
		m_alignment = Alignment::MiddleCenter;
		m_text = "Button";
		m_state = BSNormal;
	}

	void Button::onDraw(Graphics& g) {
		Rect b = bounds();

		std::string state = "normal";
		switch (m_state) {
			case BSNormal: state = "normal"; break;
			case BSHover: state = "hover"; break;
			case BSClick: state = "click"; break;
		}

		g.styledRect(b.x, b.y, b.w, b.h, app()->style()["Button"][state]);
		Label::onDraw(g);
	}

	EventStatus Button::onEvent(Event* event) {
		EventStatus status = Element::onEvent(event);
		if (event->type() == MouseEventType) {
			Rect b = intersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			switch (m_state) {
				case BSHover: {
					if (e->pressed && e->button == 1) {
						m_state = BSClick;
						status = EventStatus::Consumed;
						invalidate();
					}
				} break;
				case BSClick: {
					if (!b.hasPoint(e->x, e->y)) {
						m_state = BSNormal;
						status = EventStatus::Consumed;
						invalidate();
					}
					if (!e->pressed && e->button == 1) {
						if (m_onClick)
							m_onClick();
						m_state = BSHover;
						status = EventStatus::Consumed;
						invalidate();
					}
				} break;
				default: break;
			}
		} else if (event->type() == MotionEventType) {
			Rect b = intersectedBounds();
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			switch (m_state) {
				case BSNormal: {
					if (b.hasPoint(e->x, e->y)) {
						m_state = BSHover;
						invalidate();
						status = EventStatus::Consumed;
					}
				} break;
				case BSHover: {
					if (!b.hasPoint(e->x, e->y)) {
						m_state = BSNormal;
						invalidate();
					} else {
						status = EventStatus::Consumed;
					}
				} break;
				case BSClick: {
					if (!b.hasPoint(e->x, e->y)) {
						m_state = BSNormal;
						invalidate();
					} else {
						status = EventStatus::Consumed;
					}
				} break;
			}
		}
		return status;
	}

}
