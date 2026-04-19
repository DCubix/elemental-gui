#include "ToolButton.h"

#include "Layout.h"
#include "Application.h"

namespace tui {
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
    
    EventStatus ToolButton::OnEvent(Event *event)
    {
        EventStatus status = Element::OnEvent(event);
		if (event->Type() == EventType::MouseButton) {
			Rectangle b = GetIntersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			switch (m_state) {
				case ButtonState::Hover: {
					if (e->pressed && e->button == 1) {
						m_state = ButtonState::Click;
						status = EventStatus::Consumed;
                        if (m_mode == Mode::Toggle) {
                            m_toggled = !m_toggled;
                        } else if (m_mode == Mode::Radio) {
                            m_toggled = true;
                        }
						Invalidate();
					}
				} break;
				case ButtonState::Click: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Normal;
						status = EventStatus::Consumed;
						Invalidate();
					}
					if (!e->pressed && e->button == 1) {
						if (m_onClick)
							m_onClick();
						if (m_mode == Mode::Radio) {
							auto* prev = GetApp()->FindElement<ToolButton>([this](ToolButton* tb) {
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
						status = EventStatus::Consumed;
						Invalidate();
					}
				} break;
				default: break;
			}
		} else if (event->Type() == EventType::MouseMotion) {
			Rectangle b = GetIntersectedBounds();
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			switch (m_state) {
				case ButtonState::Normal: {
					if (b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Hover;
						Invalidate();
						status = EventStatus::Consumed;
					}
				} break;
				case ButtonState::Hover: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Normal;
						Invalidate();
					} else {
						status = EventStatus::Consumed;
					}
				} break;
				case ButtonState::Click: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Normal;
						Invalidate();
					} else {
						status = EventStatus::Consumed;
					}
				} break;
			}
		}
		return status;
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