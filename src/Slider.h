#pragma once

#include "Element.h"

namespace gui {
    constexpr int SliderThumbSize = 14;

    class Slider : public Element {
      public:
        Slider();

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "Slider"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnMouseMove(MotionEvent e) override;
        void OnScroll(ScrollEvent e) override;

        const Range& GetRange() const { return m_range; }
        void SetRange(float min, float max);

        float GetValue() const { return m_value; }
        void SetValue(float v);

        float GetStep() const { return m_step; }
        void SetStep(float s) { m_step = s; }

        Direction GetDirection() const { return m_direction; }
        void SetDirection(Direction dir) {
            m_direction = dir;
            Invalidate();
        }

        void SetOnValueChange(const ValueChanged<float>& cb) { m_onValueChange = cb; }

      private:
        enum class ButtonState { Normal = 0, Hover, Click };

        Direction m_direction;
        Range m_range;
        float m_value;
        float m_step;

        int m_thumbPos;
        int m_dragOffset;

        ButtonState m_state;

        ValueChanged<float> m_onValueChange;

        void UpdateValue(int p);
    };
} // namespace gui
