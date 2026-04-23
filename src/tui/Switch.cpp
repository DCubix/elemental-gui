#include "Switch.h"

#include "Application.h"

namespace tui {
    Switch::Switch() : Element()
    {
        SetLocalBounds(Rectangle(0, 0, 40, 22));
    }

    void Switch::OnMouseDown(MouseEvent e) {
		if (e.button != MouseButton::Left) return;
		m_pressed = true;
		Invalidate();
    }

	void Switch::OnMouseUp(MouseEvent e) {
		if (e.button != MouseButton::Left) return;
		if (m_pressed) {
			m_pressed = false;
			bool newChecked = (m_state != State::Checked);
			m_state = newChecked ? State::Checked : State::Unchecked;
			if (m_onChanged) m_onChanged(newChecked);
			Invalidate();
		}
	}

	void Switch::OnMouseEnter() {
		Invalidate();
	}

	void Switch::OnMouseLeave() {
		Invalidate();
	}

    void Switch::OnDraw(Graphics &g)
    {
        Rectangle b = GetBounds();
        Json style = GetStyle()["Switch"];

        int trackW = b.w;
        int trackH = b.h;
        int thumbRadius = (trackH - 4) / 2;
        int thumbCY = b.y + trackH / 2;

        // Draw track
        std::string trackState = (m_state == State::Checked) ? "on" : "off";
        g.StyledRect(b.x, b.y, trackW, trackH, style["track"][trackState]);

        // Thumb position: left when unchecked, center for indeterminate, right when checked
        int thumbCX;
        switch (m_state) {
            case State::Checked:
                thumbCX = b.x + trackW - thumbRadius - 2;
                break;
            case State::Indeterminate:
                thumbCX = b.x + trackW / 2;
                break;
            default:
                thumbCX = b.x + thumbRadius + 2;
                break;
        }

        // Draw thumb
        std::string thumbState = "normal";
        if (m_pressed) thumbState = "click";
        else if (m_hovered) thumbState = "hover";
        if (m_state == State::Checked) thumbState += "On";
        g.StyledRect(thumbCX - thumbRadius, thumbCY - thumbRadius,
                     thumbRadius * 2, thumbRadius * 2, style["thumb"][thumbState]);
    }

    void Switch::SetChecked(bool checked)
    {
        if (checked) {
            m_state = State::Checked;
        } else {
            m_state = State::Unchecked;
        }
        Invalidate();
    }

    void Switch::SetIndeterminate(bool indeterminate)
    {
        if (indeterminate) {
            m_state = State::Indeterminate;
        } else {
            m_state = State::Unchecked;
        }
        Invalidate();
    }
}