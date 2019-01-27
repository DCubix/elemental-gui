#ifndef TUI_SLIDER_H
#define TUI_SLIDER_H

#include "Element.h"

namespace tui {

	constexpr int SliderButtonSize = 20;
	class Slider : public Element {
	public:
		enum Orientation {
			Horizontal = 0,
			Vertical
		};

		Slider();

		virtual void onDraw(Graphics& g) override;
		virtual EventStatus onEvent(Event *event) override;

		Range& range() { invalidate(); return m_range; }
		void range(float min, float max);

		float value() const { return m_value; }
		void value(float v) { m_value = v; invalidate(); }

		float step() const { return m_step; }
		void step(float s) { m_step = s; }

		Orientation orientation() const { return m_orientation; }
		void orientation(Orientation ori) { m_orientation = ori; invalidate(); }

	private:
		enum ButtonState {
			BSNormal = 0,
			BSHover,
			BSClick
		};

		Orientation m_orientation;
		Range m_range;
		float m_value;
		float m_step;

		int m_buttonPos;

		ButtonState m_state;

		void updateValue(int p);
	};
}

#endif // TUI_SLIDER_H
