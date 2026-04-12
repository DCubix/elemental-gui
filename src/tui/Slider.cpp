#include "Slider.h"

#include "Application.h"

#include <iostream>

namespace tui {

	Slider::Slider()
		: Element(),
		  m_orientation(Horizontal),
		  m_range(Range(0, 100)),
		  m_value(0),
		  m_step(1),
		  m_state(BSNormal)
	{
		GetLocalBounds().w = 120;
		GetLocalBounds().h = 22;
	}

	void Slider::OnDraw(Graphics& g) {
		Rectangle b = GetBounds();
		int btn = GetButtonSize();
		int sz = (m_orientation == Horizontal ? b.w : b.h) - btn;
		m_buttonPos = int(m_range.Normalized(m_value) * float(sz));

		g.StyledRect(b.x, b.y, b.w, b.h, GetApp()->GetStyle()["Slider"]);

		std::string state = "normal";
		switch (m_state) {
			case BSNormal: state = "normal"; break;
			case BSHover: state = "hover"; break;
			case BSClick: state = "click"; break;
		}

//		const std::string txt = std::to_string(m_value);
//		g.styledTextBegin(app()->style()["DefaultText"]);
//		auto&& ex = g.measureText(txt);

		if (m_orientation == Horizontal) {
			g.StyledRect(b.x + m_buttonPos, b.y, btn, b.h, GetApp()->GetStyle()["Button"][state]);
//			g.styledTextEnd(txt, b.x + (b.w / 2 - int(ex.width) / 2), b.y + b.h / 2 + int(ex.height) / 2);
		} else {
			g.StyledRect(b.x, b.y + m_buttonPos, b.w, btn, GetApp()->GetStyle()["Button"][state]);
//			g.styledTextEnd(
//						txt,
//						b.x + b.w / 2 - ex.height / 2,
//						b.y + b.h / 2 - ex.width / 2,
//						PI / 2
//			);
		}
	}

	EventStatus Slider::OnEvent(Event* event) {
		EventStatus status = Element::OnEvent(event);
		if (event->Type() == EventType::MouseEventType) {
			Rectangle b = GetIntersectedBounds();
			Rectangle c = GetBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			int p = (m_orientation == Horizontal ? e->x - c.x : e->y - c.y);
			switch (m_state) {
				case BSHover: {
					if (e->pressed && e->button == 1) {
						m_state = BSClick;
						status = EventStatus::Consumed;
						Invalidate();
					}
				} break;
				case BSClick: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = BSNormal;
						status = EventStatus::Consumed;
						Invalidate();
					}
					if (!e->pressed && e->button == 1) {
						m_state = BSHover;
						status = EventStatus::Consumed;
						Invalidate();
					} else {
						UpdateValue(p);
					}
				} break;
				default: break;
			}
		} else if (event->Type() == EventType::MotionEventType) {
			Rectangle b = GetIntersectedBounds();
			Rectangle c = GetBounds();
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			int p = (m_orientation == Horizontal ? e->x - c.x : e->y - c.y);
			switch (m_state) {
				case BSNormal: {
					if (b.HasPoint(e->x, e->y)) {
						m_state = BSHover;
						status = EventStatus::Consumed;
						Invalidate();
					}
				} break;
				case BSHover: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = BSNormal;
						Invalidate();
					} else {
						status = EventStatus::Consumed;
					}
				} break;
				case BSClick: {
					UpdateValue(p);
					status = EventStatus::Consumed;
				} break;
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
		m_value = v; Invalidate();
		if (m_onValueChange) m_onValueChange();
	}

	void Slider::UpdateValue(int p) {
		int btn = GetButtonSize();
		Rectangle b = GetBounds();
		p -= btn/2;
		int sz = (m_orientation == Horizontal ? b.w : b.h) - btn;
//		if (m_orientation == Vertical) {
//			p = (sz + SliderButtonSize) - p;
//		}

		Range vp{ 0, float(sz) };
		m_value = m_range.Constrain(m_range.Remap(vp, p));
		SetValue(std::floor(m_value / m_step) * m_step);
		Invalidate();
	}

	int Slider::GetButtonSize() {
		Rectangle b = GetBounds();
		int vpSize = (m_orientation == Horizontal ? b.w : b.h);
		float contentSize = (m_range.maximum - m_range.minimum) + vpSize;
		float viewRatio = vpSize / contentSize;
		int btnSize = int(vpSize * viewRatio);
		return btnSize < SliderButtonSize ? SliderButtonSize : btnSize;
	}

}
