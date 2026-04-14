#include "Element.h"

#include <algorithm>
#include <fstream>
#include <cmath>
#include <iostream>

#include "Application.h"

namespace tui {

	Element::Element()
		: m_parent(nullptr), m_bounds(0, 0, 50, 50),
		  m_dirty(true), m_visible(true),
		  m_focused(false), m_autoSize(false),
		  m_application(nullptr)
	{}

	void Element::OnDraw(Graphics& g) {}

	EventStatus Element::OnEvent(Event* event) {
		// Handle focus on mouse click
		if (event->Type() == EventType::MouseButton) {
			Rectangle b = GetIntersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			if (b.HasPoint(e->x, e->y) && e->pressed && e->button == 1) {
				RequestFocus();
			}
		}
		return EventStatus::Active;
	}

	void Element::Invalidate() {
		m_dirty = true;
		if (m_application) m_application->RequestRedraw();
	}

	void Element::RequestFocus() {
		if (m_application) {
			m_application->Focus(this);
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
				.Intersect(GetBounds()).value_or(GetBounds());
		}
		return GetBounds();
	}

}
