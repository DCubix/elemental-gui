#include "ProgressBar.h"

namespace gui {
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
        Size size = GetSize();

        g.StyledRect(0, 0, size.w, size.h, style["track"]);

        if (!m_indeterminate) {
            float progress = m_range.Normalized(m_value);
            if (m_direction == Direction::Horizontal) {
                int pw = (int)(size.w * progress);
                g.StyledRect(0, 0, pw, size.h, style["progress"]);
            } else {
                int ph = (int)(size.h * progress);
                g.StyledRect(0, size.h - ph, size.w, ph, style["progress"]);
            }
        } else {
            g.ClipPushPath([&]() { g.GetStyledPath(style["track"], 0, 0, size.w, size.h); });
            int indSize = (m_direction == Direction::Horizontal ? size.w : size.h) / 2;
            if (m_indeterminateOffset > size.w) {
                m_indeterminateOffset = -indSize;
            }

            if (m_direction == Direction::Horizontal) {
                g.StyledRect(m_indeterminateOffset, 0, indSize, size.h, style["indeterminate"]);
            } else {
                g.StyledRect(0, m_indeterminateOffset, size.w, indSize, style["indeterminate"]);
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