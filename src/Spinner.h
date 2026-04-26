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

        float GetValue() const { return m_value; }
        void SetValue(float v);

        float GetStep() const { return m_step; }
        void SetStep(float s) { m_step = s; }

        int GetDecimals() const { return m_decimals; }
        void SetDecimals(int d) {
            m_decimals = d;
            Invalidate();
        }

        void SetOnValueChange(const ValueChanged<float>& cb) { m_onValueChange = cb; }

    private:
        enum class PartState { Normal, Hover, Click };

        Range m_range{0.0f, 100.0f};
        float m_value{0.0f};
        float m_step{1.0f};
        int m_decimals{0};

        PartState m_upState{PartState::Normal};
        PartState m_downState{PartState::Normal};

        ValueChanged<float> m_onValueChange;

        Rectangle GetUpButtonRect() const;
        Rectangle GetDownButtonRect() const;
        std::string FormatValue() const;
        void Step(float direction);
    };
} // namespace gui
