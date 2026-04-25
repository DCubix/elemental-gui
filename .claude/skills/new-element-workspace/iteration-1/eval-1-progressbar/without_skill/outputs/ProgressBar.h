#pragma once

#include "Element.h"
#include "Timer.h"

namespace gui {
    class ProgressBar : public Element {
    public:
        ProgressBar();

        void OnDraw(Graphics& g) override;

        const Range& GetRange() const { return m_range; }
        void SetRange(float min, float max);

        float GetValue() const { return m_value; }
        void SetValue(float v);

        Direction GetDirection() const { return m_direction; }
        void SetDirection(Direction dir) { m_direction = dir; Invalidate(); }

        bool IsIndeterminate() const { return m_indeterminate; }
        void SetIndeterminate(bool indeterminate);

    private:
        Range m_range;
        float m_value{0.0f};
        bool m_indeterminate{false};
        Direction m_direction{Direction::Horizontal};
        Timer m_timer;
        int m_indeterminateOffset{0};
    };
}
