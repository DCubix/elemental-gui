#include "ProgressBar.h"

namespace tui {
    ProgressBar::ProgressBar() : Element()
    {
        m_range.minimum = 0.0f;
        m_range.maximum = 100.0f;
        m_value = 0.0f;
        m_indeterminate = false;
        SetLocalBounds({ 0, 0, 100, 20 });
    }

    void ProgressBar::OnDraw(Graphics &g)
    {
        auto style = GetStyle()["ProgressBar"];

        // Track
        Rectangle b = GetBounds();
        g.StyledRect(b.x, b.y, b.w, b.h, style["track"]);

        if (!m_indeterminate) {
            // Progress
            float progress = m_range.Normalized(m_value);
            
            if (m_direction == Direction::Horizontal) {
                int pw = (int)(b.w * progress);
                g.StyledRect(b.x, b.y, pw, b.h, style["progress"]);
            } else {
                int ph = (int)(b.h * progress);
                g.StyledRect(b.x, b.y + b.h - ph, b.w, ph, style["progress"]);
            }
        } else {
            g.ClipPushPath([&]() { g.GetStyledPath(style["track"], b.x, b.y, b.w, b.h); });
            int indSize = (m_direction == Direction::Horizontal ? b.w : b.h) / 2;
            if (m_indeterminateOffset > b.w) {
                m_indeterminateOffset = -indSize;
            }

            if (m_direction == Direction::Horizontal) {
                g.StyledRect(b.x + m_indeterminateOffset, b.y, indSize, b.h, style["indeterminate"]);
            } else {
                g.StyledRect(b.x, b.y + m_indeterminateOffset, b.w, indSize, style["indeterminate"]);
            }
            g.ClipPop();
        }
    }

    void ProgressBar::SetRange(float min, float max)
    {
        m_range.minimum = min;
        m_range.maximum = max;
        Invalidate();
    }
    
    void ProgressBar::SetValue(float v)
    {
        m_value = v;
        Invalidate();
    }
    
    void ProgressBar::SetIndeterminate(bool indeterminate)
    {
        m_indeterminate = indeterminate;
        if (indeterminate) {
            int size = (m_direction == Direction::Horizontal ? GetBounds().w : GetBounds().h) / 2;
            m_indeterminateOffset = -size;
            m_timer.Start(30, [this]() {
                m_indeterminateOffset += 5;
                Invalidate();
            });
        } else {
            m_timer.Stop();
        }
        Invalidate();
    }
}