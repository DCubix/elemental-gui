#pragma once

#include "Element.h"

namespace gui {
    constexpr int ScrollbarButtonSize = 16;
    class Scrollbar : public Element {
    public:
        Scrollbar();

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "Scrollbar"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnMouseMove(MotionEvent e) override;
        void OnScroll(ScrollEvent e) override;

        Range& GetRange() {
            Invalidate();
            return m_range;
        }
        void SetRange(float min, float max);

        Property<float> value{0.0f};
        Property<float> step{1.0f};
        Property<Direction> direction{Direction::Horizontal};

    private:
        Range m_range;

        int m_buttonPos;
        int m_dragOffset;

        ButtonState m_state;

        void UpdateValue(int p);
        int GetButtonSize();
    };
} // namespace gui
