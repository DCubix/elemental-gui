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
		localBounds().w = 120;
		localBounds().h = 22;
	}

	void Slider::onDraw(Graphics& g) {
		Rect b = bounds();
		int btn = getButtonSize();
		int sz = (m_orientation == Horizontal ? b.w : b.h) - btn;
		m_buttonPos = int(m_range.normalized(m_value) * float(sz));

		g.styledRect(b.x, b.y, b.w, b.h, app()->style()["Slider"]);

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
			g.styledRect(b.x + m_buttonPos, b.y, btn, b.h, app()->style()["Button"][state]);
//			g.styledTextEnd(txt, b.x + (b.w / 2 - int(ex.width) / 2), b.y + b.h / 2 + int(ex.height) / 2);
		} else {
			g.styledRect(b.x, b.y + m_buttonPos, b.w, btn, app()->style()["Button"][state]);
//			g.styledTextEnd(
//						txt,
//						b.x + b.w / 2 - ex.height / 2,
//						b.y + b.h / 2 - ex.width / 2,
//						PI / 2
//			);
		}
	}

	EventStatus Slider::onEvent(Event* event) {
		EventStatus status = Element::onEvent(event);
		if (event->type() == MouseEventType) {
			Rect b = intersectedBounds();
			Rect c = bounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			int p = (m_orientation == Horizontal ? e->x - c.x : e->y - c.y);
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
						m_state = BSHover;
						status = EventStatus::Consumed;
						invalidate();
					} else {
						updateValue(p);
					}
				} break;
				default: break;
			}
		} else if (event->type() == MotionEventType) {
			Rect b = intersectedBounds();
			Rect c = bounds();
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			int p = (m_orientation == Horizontal ? e->x - c.x : e->y - c.y);
			switch (m_state) {
				case BSNormal: {
					if (b.hasPoint(e->x, e->y)) {
						m_state = BSHover;
						status = EventStatus::Consumed;
						invalidate();
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
					updateValue(p);
					status = EventStatus::Consumed;
				} break;
			}
		}
		return status;
	}

	void Slider::range(float min, float max) {
		m_range.minimum = min;
		m_range.maximum = max;
		value(m_range.constrain(m_value));
		invalidate();
	}

	void Slider::value(float v) {
		m_value = v; invalidate();
		if (m_onValueChange) m_onValueChange();
	}

	void Slider::updateValue(int p) {
		int btn = getButtonSize();
		Rect b = bounds();
		p -= btn/2;
		int sz = (m_orientation == Horizontal ? b.w : b.h) - btn;
//		if (m_orientation == Vertical) {
//			p = (sz + SliderButtonSize) - p;
//		}

		Range vp{ 0, float(sz) };
		m_value = m_range.constrain(m_range.remap(vp, p));
		value(std::floor(m_value / m_step) * m_step);
		invalidate();
	}

	int Slider::getButtonSize() {
		Rect b = bounds();
		int vpSize = (m_orientation == Horizontal ? b.w : b.h);
		float contentSize = (m_range.maximum - m_range.minimum) + vpSize;
		float viewRatio = vpSize / contentSize;
		int btnSize = int(vpSize * viewRatio);
		return btnSize < SliderButtonSize ? SliderButtonSize : btnSize;
	}

}
