#pragma once

#include "Element.h"
#include "Timer.h"

namespace gui {
    class ProgressBar : public Element {
    public:
        ProgressBar();

        virtual void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "ProgressBar"; }

        const Range& GetRange() const { return m_range; }
        void SetRange(float min, float max);

        bool IsIndeterminate() const { return m_indeterminate; }
        void SetIndeterminate(bool indeterminate);

        Property<float> value{0.0f};
        Property<Direction> direction{Direction::Horizontal};

    private:
        Range m_range;
        bool m_indeterminate{false};
        Timer m_timer;
        int m_indeterminateOffset{0};
    };
} // namespace gui