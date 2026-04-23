#include "Element.h"

#include <cmath>

#include "Application.h"
#include "EventSystem.h"
#include "Window.h"

namespace tui {

	Element::Element()
		: m_parent(nullptr), m_bounds(0, 0, 50, 50),
		  m_dirty(true), m_visible(true),
		  m_focused(false), m_autoSize(false)
	{}

	void Element::OnDraw(Graphics& g) {}

	EventStatus Element::OnEvent(Event* event) {
		Rectangle intersectedBounds = GetIntersectedBounds();
		Rectangle localBounds = GetBounds();

		// Handle focus on mouse click
		if (event->Type() == EventType::MouseButton) {
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			if (intersectedBounds.HasPoint(e->x, e->y) &&
				e->pressed && e->button == MouseButton::Left) {
				RequestFocus();
			}
		}

		switch (event->Type()) {
			case EventType::MouseButton: {
				MouseEvent eCopy = *dynamic_cast<MouseEvent*>(event);
				if (!intersectedBounds.HasPoint(eCopy.x, eCopy.y)) {
					return EventStatus::Active;
				}

				eCopy.x -= localBounds.x;
				eCopy.y -= localBounds.y;
				
				if (eCopy.pressed) {
					OnMouseDown(eCopy);
				} else {
					OnMouseUp(eCopy);
				}
				return EventStatus::Consumed;
			};
			case EventType::MouseMotion: {
				MotionEvent eCopy = *dynamic_cast<MotionEvent*>(event);

				if (!intersectedBounds.HasPoint(eCopy.x, eCopy.y)) {
					if (m_hovered) {
						m_hovered = false;
						OnMouseLeave();
					}
					return EventStatus::Active;
				}

				if (!m_hovered) {
					m_hovered = true;
					OnMouseEnter();
				}

				OnMouseMove(eCopy);
				return EventStatus::Consumed;
			};
			case EventType::Scroll: {
				ScrollEvent eCopy = *dynamic_cast<ScrollEvent*>(event);
				if (!intersectedBounds.HasPoint(eCopy.mouseX, eCopy.mouseY)) {
					return EventStatus::Active;
				}
				eCopy.mouseX -= localBounds.x;
				eCopy.mouseY -= localBounds.y;
				OnScroll(eCopy);
				return EventStatus::Consumed;
			};
			case EventType::Key: {
				KeyEvent eCopy = *dynamic_cast<KeyEvent*>(event);
				if (eCopy.pressed) {
					OnKeyDown(eCopy);
				} else {
					OnKeyUp(eCopy);
				}
				return EventStatus::Consumed;
			}
			case EventType::TextInput: {
				TextInputEvent eCopy = *dynamic_cast<TextInputEvent*>(event);
				OnTextInput(eCopy);
				return EventStatus::Consumed;
			}
			case EventType::Focus: {
				FocusEvent eCopy = *dynamic_cast<FocusEvent*>(event);
				OnFocus(eCopy);
				return EventStatus::Consumed;
			}
			case EventType::Blur: {
				BlurEvent eCopy = *dynamic_cast<BlurEvent*>(event);
				OnBlur(eCopy);
				return EventStatus::Consumed;
			}
		}
		
		return EventStatus::Active;
	}

	void Element::Invalidate() {
		m_dirty = true;
		if (m_window) m_window->RequestRedraw();
	}

	void Element::RequestFocus() {
		if (m_window) {
			m_window->Focus(this);
			Invalidate();
		}
	}

    Json Element::GetStyle() const
    {
		if (m_style.is_null()) {
			return Application::DefaultStyle;
		}
		return m_style;
    }

    Rectangle Element::GetBounds() const {
		Rectangle b = m_bounds;
		if (m_parent != nullptr) {
			b.x += m_parent->GetBounds().x;
			b.y += m_parent->GetBounds().y;
		}
		return b;
	}

	Rectangle Element::GetIntersectedBounds() const {
		if (m_parent != nullptr) {
			// Returns only the visible part of the element
			// by intersecting it with the parent's intersected bounds
			return m_parent->GetIntersectedBounds()
				.Intersect(GetBounds()).value_or(Rectangle(0, 0, 0, 0));
		}
		return GetBounds();
	}

}
