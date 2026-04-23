#include "RadioButton.h"

#include "Application.h"

namespace tui {
    RadioButton::RadioButton() : Element(), m_text("") {
		SetLocalBounds(Rectangle(0, 0, 100, 22));
	}
    
    void RadioButton::OnMouseDown(MouseEvent e) {
		if (e.button != MouseButton::Left) return;
		m_pressed = true;
		Invalidate();
    }

	void RadioButton::OnMouseUp(MouseEvent e) {
		if (e.button != MouseButton::Left) return;
		if (m_pressed) {
			m_pressed = false;

			auto* prev = GetApp()->FindElement<RadioButton>([this](RadioButton* rb) {
				return rb != this && rb->GetGroup() == GetGroup();
			});
			if (prev) {
				prev->SetChecked(false);
			}
			SetChecked(true);

			Invalidate();
		}
	}

	void RadioButton::OnMouseEnter() {
		Invalidate();
	}

	void RadioButton::OnMouseLeave() {
		Invalidate();
	}
    
    void RadioButton::OnDraw(Graphics &g)
    {
        Rectangle b = GetBounds();
        Json style = GetStyle()["RadioButton"];
        Json textStyle = GetStyle()["DefaultText"];

        int circleSize = style.value("size", b.h);
        int circleX = b.x;
        int circleY = b.y + (b.h - circleSize) / 2;
        int textX = b.x + circleSize + 6;
        int textY = b.y + b.h / 2;

        // Select state-based style
        std::string circleState = "normal";
        if (m_checked) {
            circleState = (m_hovered && !m_pressed) ? "selectedHover" : "selected";
        } else if (m_pressed) {
            circleState = "click";
        } else if (m_hovered) {
            circleState = "hover";
        } else {
            circleState = "normal";
        }

        // Draw the radio button circle
        g.StyledRect(circleX, circleY, circleSize, circleSize, style[circleState]);

        // Draw inner filled circle when checked
        if (m_checked && style["dot"].is_object()) {
            int dotSize = circleSize / 2;
            int dotX = circleX + (circleSize - dotSize) / 2;
            int dotY = circleY + (circleSize - dotSize) / 2;
            g.StyledRect(dotX, dotY, dotSize, dotSize, style["dot"]);
        }

        // Draw label text
        if (!m_text.empty()) {
            int textX = circleX + circleSize + 6;
			int textY = circleY;

			g.StyledTextBegin(textStyle);
			auto ex = g.MeasureText(m_text);
			int textOffY = circleSize / 2 + static_cast<int>(ex.height) / 2;
			g.StyledTextEnd(m_text, textX, textY + textOffY);
        }
    }
    
    Size RadioButton::GetPreferredSize() const
    {
        Json style = GetStyle()["RadioButton"];
        int circleSize = style.value("size", m_bounds.h);
        return { m_bounds.w, circleSize };
    }

    void RadioButton::SetChecked(bool checked)
    {
        if (m_checked != checked) {
            m_checked = checked;
            if (m_onChanged) {
                m_onChanged(m_checked);
            }
            Invalidate();
        }
    }
}