#include "Spinner.h"
#include "Application.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace gui {

    Spinner::Spinner() : Element() {
        SetLocalBounds(Rectangle(0, 0, 90, 24));
    }

    Rectangle Spinner::GetUpButtonRect() const {
        Rectangle b = GetBounds();
        return Rectangle(b.x + b.w - SpinnerButtonWidth, b.y, SpinnerButtonWidth, b.h / 2);
    }

    Rectangle Spinner::GetDownButtonRect() const {
        Rectangle b = GetBounds();
        int half = b.h / 2;
        return Rectangle(b.x + b.w - SpinnerButtonWidth, b.y + half, SpinnerButtonWidth, b.h - half);
    }

    std::string Spinner::FormatValue() const {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(m_decimals) << m_value;
        return ss.str();
    }

    void Spinner::Step(float direction) {
        float snapped = std::round(m_value / m_step) * m_step;
        SetValue(m_range.Constrain(snapped + direction * m_step));
    }

    void Spinner::OnDraw(Graphics& g) {
        Rectangle b = GetBounds();
        Json style = GetStyle()["Spinner"];
        Json textStyle = GetStyle()["DefaultText"];

        // Draw background/border
        g.StyledRect(b.x, b.y, b.w, b.h, style["normal"]);

        // Draw value text, clipped to text area
        int textAreaW = b.w - SpinnerButtonWidth - 12;
        g.ClipPushRect(b.x + 6, b.y, textAreaW, b.h);
        g.StyledTextBegin(textStyle);
        auto te = g.MeasureText(FormatValue());
        g.StyledTextEnd(FormatValue(), b.x + 6, b.y + (b.h + (int)te.height) / 2);
        g.ClipPop();

        // Vertical divider
        int divX = b.x + b.w - SpinnerButtonWidth;
        g.Color(0.25f, 0.25f, 0.28f, 1.0f);
        g.LineWidth(1.0f);
        g.Line(divX, b.y + 3, divX, b.y + b.h - 3);
        g.Stroke();

        // Up button highlight
        Rectangle upRect = GetUpButtonRect();
        if (m_upState != PartState::Normal) {
            std::string s = m_upState == PartState::Click ? "click" : "hover";
            g.StyledRect(upRect.x, upRect.y, upRect.w, upRect.h, style["button"][s]);
        }

        // Down button highlight
        Rectangle downRect = GetDownButtonRect();
        if (m_downState != PartState::Normal) {
            std::string s = m_downState == PartState::Click ? "click" : "hover";
            g.StyledRect(downRect.x, downRect.y, downRect.w, downRect.h, style["button"][s]);
        }

        // Arrow color
        g.Color(0.72f, 0.72f, 0.76f, 1.0f);

        // Up arrow (triangle pointing up)
        {
            int cx = upRect.x + upRect.w / 2;
            int cy = upRect.y + upRect.h / 2;
            g.BeginPath();
            g.AddPathPoint(cx - 4, cy + 2);
            g.AddPathPoint(cx,     cy - 2);
            g.AddPathPoint(cx + 4, cy + 2);
            g.EndPath(true);
            g.Fill();
        }

        // Down arrow (triangle pointing down)
        {
            int cx = downRect.x + downRect.w / 2;
            int cy = downRect.y + downRect.h / 2;
            g.BeginPath();
            g.AddPathPoint(cx - 4, cy - 2);
            g.AddPathPoint(cx,     cy + 2);
            g.AddPathPoint(cx + 4, cy - 2);
            g.EndPath(true);
            g.Fill();
        }
    }

    void Spinner::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left) return;

        if (GetUpButtonRect().HasPoint(e.x, e.y)) {
            m_upState = PartState::Click;
            Step(+1.0f);
        } else if (GetDownButtonRect().HasPoint(e.x, e.y)) {
            m_downState = PartState::Click;
            Step(-1.0f);
        }
        Invalidate();
    }

    void Spinner::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        if (m_upState == PartState::Click) {
            m_upState = GetUpButtonRect().HasPoint(e.x, e.y) ? PartState::Hover : PartState::Normal;
            Invalidate();
        }
        if (m_downState == PartState::Click) {
            m_downState = GetDownButtonRect().HasPoint(e.x, e.y) ? PartState::Hover : PartState::Normal;
            Invalidate();
        }
    }

    void Spinner::OnMouseEnter() {
        Invalidate();
    }

    void Spinner::OnMouseLeave() {
        m_upState = PartState::Normal;
        m_downState = PartState::Normal;
        Invalidate();
    }

    void Spinner::OnMouseMove(MotionEvent e) {
        PartState newUp   = GetUpButtonRect().HasPoint(e.x, e.y)   ? PartState::Hover : PartState::Normal;
        PartState newDown = GetDownButtonRect().HasPoint(e.x, e.y)  ? PartState::Hover : PartState::Normal;

        if (m_upState != PartState::Click && newUp != m_upState)     { m_upState = newUp;     Invalidate(); }
        if (m_downState != PartState::Click && newDown != m_downState) { m_downState = newDown; Invalidate(); }
    }

    void Spinner::OnScroll(ScrollEvent e) {
        Step(e.scrollY > 0 ? +1.0f : -1.0f);
    }

    Size Spinner::GetPreferredSize() const {
        if (IsAutoSize()) return { 90, 24 };
        return Element::GetPreferredSize();
    }

    void Spinner::SetRange(float min, float max) {
        m_range.minimum = min;
        m_range.maximum = max;
        SetValue(m_range.Constrain(m_value));
        Invalidate();
    }

    void Spinner::SetValue(float v) {
        float clamped = m_range.Constrain(v);
        if (m_onValueChange && m_value != clamped) m_onValueChange(clamped);
        m_value = clamped;
        Invalidate();
    }

}
