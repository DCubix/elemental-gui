#include "ToggleSwitch.h"
#include "Application.h"

namespace tui {
    ToggleSwitch::ToggleSwitch() : Element() {
        SetLocalBounds(Rectangle(0, 0, 52, 28));
    }

    void ToggleSwitch::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        m_pressed = true;
        Invalidate();
    }

    void ToggleSwitch::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        if (m_pressed) {
            m_pressed = false;
            m_on = !m_on;
            if (m_onChanged) m_onChanged(m_on);
            Invalidate();
        }
    }

    void ToggleSwitch::OnMouseEnter() {
        Invalidate();
    }

    void ToggleSwitch::OnMouseLeave() {
        Invalidate();
    }

    void ToggleSwitch::OnDraw(Graphics& g) {
        Rectangle b = GetBounds();
        Json style = GetStyle()["ToggleSwitch"];
        Json textStyle = GetStyle()["DefaultText"];

        int trackW = style.value("width", 52);
        int trackH = style.value("height", 28);
        int trackX = b.x;
        int trackY = b.y + (b.h - trackH) / 2;

        int thumbPad = 3;
        int thumbDiam = trackH - thumbPad * 2;
        int thumbCY = trackY + trackH / 2;

        // Draw track
        std::string trackState = m_on ? "on" : "off";
        g.StyledRect(trackX, trackY, trackW, trackH, style["track"][trackState]);

        // Thumb position: left = off, right = on
        int thumbCX = m_on
            ? trackX + trackW - thumbPad - thumbDiam / 2
            : trackX + thumbPad + thumbDiam / 2;

        // Thumb state
        std::string thumbState;
        if (m_pressed)      thumbState = m_on ? "clickOn" : "click";
        else if (m_hovered) thumbState = m_on ? "hoverOn"  : "hover";
        else                thumbState = m_on ? "normalOn" : "normal";

        int tx = thumbCX - thumbDiam / 2;
        int ty = thumbCY - thumbDiam / 2;
        g.StyledRect(tx, ty, thumbDiam, thumbDiam, style["thumb"][thumbState]);

        // Optional label to the right of the track
        if (!m_label.empty()) {
            int textX = trackX + trackW + 8;
            int textY = b.y;

            g.StyledTextBegin(textStyle);
            auto ex = g.MeasureText(m_label);
            int textOffY = b.h / 2 + static_cast<int>(ex.height) / 2;
            g.StyledTextEnd(m_label, textX, textY + textOffY);
        }
    }

    Size ToggleSwitch::GetPreferredSize() const {
        Json style = GetStyle()["ToggleSwitch"];
        int h = style.value("height", m_bounds.h);
        return { m_bounds.w, h };
    }

    void ToggleSwitch::SetOn(bool on) {
        m_on = on;
        Invalidate();
    }
}
