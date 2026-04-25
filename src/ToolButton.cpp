#include "ToolButton.h"

#include "Layout.h"
#include "Window.h"

namespace gui {
    ToolButton::ToolButton() : Label()
    {
        SetLocalBounds(Rectangle(0, 0, 50, 22));
		m_alignment = Alignment::MiddleCenter;
		m_text = "Button";
		m_state = ButtonState::Normal;
    }
    
    void ToolButton::OnDraw(Graphics &g)
    {
        Rectangle b = GetBounds();

		EdgeInsets pad = EdgeInsets::FromStyle(
            GetStyle()["ToolButton"]["padding"]
        );

		std::string state = "";
		switch (m_state) {
			case ButtonState::Normal: state = ""; break;
			case ButtonState::Hover: state = "hover"; break;
			case ButtonState::Click: state = "click"; break;
		}

        if (m_toggled) {
            state = "click";
        }

        if (state != "") {
            g.StyledRect(
                b.x,
                b.y,
                b.w,
                b.h,
                GetStyle()["ToolButton"][state]
            );
        }
		Label::OnDraw(g);
    }
    
    void ToolButton::OnMouseDown(MouseEvent e) {
		if (e.button != MouseButton::Left) return;
		if (m_state == ButtonState::Hover) {
			m_state = ButtonState::Click;
			if (m_mode == Mode::Toggle) {
				m_toggled = !m_toggled;
			} else if (m_mode == Mode::Radio) {
				m_toggled = true;
			}
			Invalidate();
		}
    }

	void ToolButton::OnMouseUp(MouseEvent e) {
		if (e.button != MouseButton::Left) return;
		if (m_state == ButtonState::Click) {
			if (m_onClick)
				m_onClick();
			if (m_mode == Mode::Radio) {
				auto* prev = m_window->Find<ToolButton>([this](ToolButton* tb) {
					return tb != this &&
						tb->GetGroup() == GetGroup() &&
						tb->GetMode() == Mode::Radio &&
						tb->IsToggled();
				});
				if (prev) {
					prev->SetToggled(false);
				}
			}
			m_state = ButtonState::Hover;
			Invalidate();
		}
	}

	void ToolButton::OnMouseEnter() {
		if (m_state == ButtonState::Normal) {
			m_state = ButtonState::Hover;
			Invalidate();
		}
	}

	void ToolButton::OnMouseLeave() {
		if (m_state == ButtonState::Hover) {
			m_state = ButtonState::Normal;
			Invalidate();
		} else if (m_state == ButtonState::Click) {
			m_state = ButtonState::Normal;
			Invalidate();
		}
	}
    
    Size ToolButton::GetPreferredSize() const
    {
        if (IsAutoSize()) {
			Size size = Label::GetPreferredSize();
			EdgeInsets padding = EdgeInsets::FromStyle(
                GetStyle()["ToolButton"]["padding"]
            );
			size.w += padding.left + padding.right;
			size.h += padding.top + padding.bottom;
			return size;
		}
		return Element::GetPreferredSize();
    }
}