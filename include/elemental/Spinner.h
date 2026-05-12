#pragma once

#include "Element.h"

namespace gui {
    constexpr int SpinnerButtonWidth = 20;

    class Spinner : public Element {
    public:
        Spinner();

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "Spinner"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnMouseMove(MotionEvent e) override;
        void OnScroll(ScrollEvent e) override;

        Size GetPreferredSize() const override;

        const Range& GetRange() const { return m_range; }
        void SetRange(float min, float max);

        Property<float> value{0.0f};
        Property<float> step{1.0f};
        Property<int> decimals{0};

    private:
        enum class PartState { Normal, Hover, Click };

        Range m_range{0.0f, 100.0f};

        PartState m_upState{PartState::Normal};
        PartState m_downState{PartState::Normal};

        Rectangle GetUpButtonRect() const;
        Rectangle GetDownButtonRect() const;
        std::string FormatValue() const;
        void Step(float direction);
    };
} // namespace gui
