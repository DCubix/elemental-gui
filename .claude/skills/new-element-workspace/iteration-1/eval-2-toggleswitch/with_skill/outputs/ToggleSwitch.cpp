#include "ToggleSwitch.h"
#include "Application.h"

namespace gui {

    ToggleSwitch::ToggleSwitch() : Element() {
        SetLocalBounds(Rectangle(0, 0, 48, 26));
    }

    void ToggleSwitch::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        if (m_state == State::Hover) {
            m_state = State::Click;
            Invalidate();
        }
    }

    void ToggleSwitch::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        if (m_state == State::Click) {
            m_on = !m_on;
            if (m_onChanged) m_onChanged(m_on);
            m_state = State::Hover;
            Invalidate();
        }
    }

    void ToggleSwitch::OnMouseEnter() {
        if (m_state == State::Normal) {
            m_state = State::Hover;
            Invalidate();
        }
    }

    void ToggleSwitch::OnMouseLeave() {
        if (m_state != State::Normal) {
            m_state = State::Normal;
            Invalidate();
        }
    }

    void ToggleSwitch::OnDraw(Graphics& g) {
        Rectangle b = GetBounds();
        Json style = GetStyle()["ToggleSwitch"];

        int trackW = b.w;
        int trackH = b.h;
        int thumbRadius = (trackH - 4) / 2;
        int thumbCY = b.y + trackH / 2;

        // Draw track based on on/off state
        std::string trackState = m_on ? "on" : "off";
        g.StyledRect(b.x, b.y, trackW, trackH, style["track"][trackState]);

        // Thumb position: left when off, right when on
        int thumbCX = m_on
            ? b.x + trackW - thumbRadius - 2
            : b.x + thumbRadius + 2;

        // Determine thumb style state
        std::string thumbState = "normal";
        switch (m_state) {
            case State::Normal: thumbState = "normal"; break;
            case State::Hover:  thumbState = "hover";  break;
            case State::Click:  thumbState = "click";  break;
        }
        if (m_on) thumbState += "On";

        g.StyledRect(
            thumbCX - thumbRadius, thumbCY - thumbRadius,
            thumbRadius * 2, thumbRadius * 2,
            style["thumb"][thumbState]
        );
    }

    Size ToggleSwitch::GetPreferredSize() const {
        if (IsAutoSize()) {
            return { 48, 26 };
        }
        return Element::GetPreferredSize();
    }

    void ToggleSwitch::SetOn(bool on) {
        m_on = on;
        Invalidate();
    }

}
