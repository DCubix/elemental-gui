#include "CheckBox.h"

#include "Application.h"

namespace tui {

	CheckBox::CheckBox() : Element(), m_text("") {
		SetLocalBounds(Rectangle(0, 0, 100, 22));
	}

	void CheckBox::OnMouseDown(MouseEvent e) {
		if (e.button != 1) return;
		m_pressed = true;
		Invalidate();
	}

	void CheckBox::OnMouseUp(MouseEvent e) {
		if (e.button != 1) return;
		if (m_pressed) {
			m_pressed = false;
			m_checked = !m_checked;
			if (m_onChanged) m_onChanged(m_checked);
			Invalidate();
		}
	}

	void CheckBox::OnMouseEnter() {
		Invalidate();
	}

	void CheckBox::OnMouseLeave() {
		Invalidate();
	}

	void CheckBox::OnDraw(Graphics &g) {
		Rectangle b = GetBounds();
		Json style = GetStyle()["CheckBox"];
		Json textStyle = GetStyle()["DefaultText"];

		int boxSize = style.value("size", b.h);
		int boxX = b.x;
		int boxY = b.y + (b.h - boxSize) / 2;

		// Select state-based style
		std::string boxState = "normal";
		if (m_checked) {
			boxState = (m_hovered && !m_pressed) ? "checkedHover" : "checked";
		} else if (m_pressed) {
			boxState = "click";
		} else if (m_hovered) {
			boxState = "hover";
		}

		// Draw the checkbox box
		g.StyledRect(boxX, boxY, boxSize, boxSize, style[boxState]);

		// Draw checkmark when checked
		if (m_checked && style["checkmark"].is_object()) {
			g.DrawSVG(style["checkmark"], boxX, boxY, boxSize, boxSize);
		}

		// Draw label text
		if (!m_text.empty()) {
			int textX = boxX + boxSize + 6;
			int textY = boxY;

			g.StyledTextBegin(textStyle);
			auto ex = g.MeasureText(m_text);
			int textOffY = boxSize / 2 + static_cast<int>(ex.height) / 2;
			g.StyledTextEnd(m_text, textX, textY + textOffY);
		}
	}

	Size CheckBox::GetPreferredSize() const {
		Json style = GetStyle()["CheckBox"];
		int boxSize = style.value("size", m_bounds.h);
		return { m_bounds.w, boxSize };
	}

	void CheckBox::SetChecked(bool checked) {
		m_checked = checked;
		Invalidate();
	}
}
