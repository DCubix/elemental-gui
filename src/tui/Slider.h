#ifndef TUI_SLIDER_H
#define TUI_SLIDER_H

#include "Element.h"

namespace tui {
	using ValueCallback = std::function<void()>;

	constexpr int SliderButtonSize = 16;
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
		void value(float v);

		float step() const { return m_step; }
		void step(float s) { m_step = s; }

		Orientation orientation() const { return m_orientation; }
		void orientation(Orientation ori) { m_orientation = ori; invalidate(); }

		void onValueChange(ValueCallback cb) { m_onValueChange = cb; }

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

		ValueCallback m_onValueChange;

		void updateValue(int p);
		int getButtonSize();
	};
}

#endif // TUI_SLIDER_H
