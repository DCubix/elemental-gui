#include "CheckBox.h"

#include "Application.h"

namespace tui {

	CheckBox::CheckBox() : Element(), m_text("") {
		SetLocalBounds(Rectangle(0, 0, 100, 22));
	}

	EventStatus CheckBox::OnEvent(Event *event) {
		EventStatus status = Element::OnEvent(event);
		if (event->Type() == EventType::MouseButton) {
			Rectangle b = GetIntersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			if (b.HasPoint(e->x, e->y)) {
				if (e->pressed && e->button == 1) {
					m_pressed = true;
					status = EventStatus::Consumed;
					Invalidate();
				} else if (!e->pressed && e->button == 1 && m_pressed) {
					m_pressed = false;
					m_checked = !m_checked;
					if (m_onChanged) m_onChanged(m_checked);
					status = EventStatus::Consumed;
					Invalidate();
				}
			} else {
				if (m_pressed) {
					m_pressed = false;
					Invalidate();
				}
			}
		} else if (event->Type() == EventType::MouseMotion) {
			Rectangle b = GetIntersectedBounds();
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			bool inside = b.HasPoint(e->x, e->y);
			if (inside != m_hovered) {
				m_hovered = inside;
				Invalidate();
			}
			if (inside) {
				status = EventStatus::Consumed;
			}
		}
		return status;
	}

	void CheckBox::OnDraw(Graphics &g) {
		Rectangle b = GetBounds();
		Json style = GetStyle()["CheckBox"];
		Json textStyle = GetStyle()["DefaultText"];

		int boxSize = b.h;
		int boxX = b.x;
		int boxY = b.y;

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
		return { m_bounds.w, m_bounds.h };
	}

	void CheckBox::SetChecked(bool checked) {
		m_checked = checked;
		Invalidate();
	}
}
