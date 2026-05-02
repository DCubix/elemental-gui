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

        Property<float> value{0.0f};
        Property<float> step{1.0f};
        Property<Direction> direction{Direction::Horizontal};

    private:
        enum class ButtonState { Normal = 0, Hover, Click };

        Range m_range;

        int m_thumbPos;
        int m_dragOffset;

        ButtonState m_state;

        void UpdateValue(int p);
    };
} // namespace gui
