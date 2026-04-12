#pragma once

#include "Element.h"

namespace tui {
	using ValueCallback = std::function<void()>;

	constexpr int SliderThumbSize = 14;

	class Slider : public Element {
	public:
		enum Orientation {
			Horizontal = 0,
			Vertical
		};

		Slider();

		virtual void OnDraw(Graphics& g) override;
		virtual EventStatus OnEvent(Event *event) override;

		Range& GetRange() { Invalidate(); return m_range; }
		void SetRange(float min, float max);

		float GetValue() const { return m_value; }
		void SetValue(float v);

		float GetStep() const { return m_step; }
		void SetStep(float s) { m_step = s; }

		Slider::Orientation GetOrientation() const { return m_orientation; }
		void SetOrientation(Orientation ori) { m_orientation = ori; Invalidate(); }

		void SetOnValueChange(ValueCallback cb) { m_onValueChange = cb; }

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

		int m_thumbPos;
		int m_dragOffset;

		ButtonState m_state;

		ValueCallback m_onValueChange;

		void UpdateValue(int p);
	};
}
