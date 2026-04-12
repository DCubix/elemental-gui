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
		if (event->Type() == EventType::MouseEventType) {
			Rectangle b = GetIntersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			if (b.HasPoint(e->x, e->y) && e->pressed && e->button == 1) {
				RequestFocus();
			}
		}
		return EventStatus::Active;
	}

	Size Element::GetPreferredSize() {
		return { m_bounds.w, m_bounds.h };
	}

	void Element::Invalidate() { m_dirty = true; if (GetApp()) GetApp()->RequestRedraw(); }

	void Element::RequestFocus() {
		if (GetApp()) {
			GetApp()->Focus(this);
			Invalidate();
		}
	}

	Rectangle Element::GetBounds() const {
		Rectangle b = m_bounds;
		if (m_parent != nullptr) {
			b.x += m_parent->GetBounds().x;
			b.y += m_parent->GetBounds().y;
		}
		return b;
	}

	Rectangle Element::GetIntersectedBounds() {
		if (m_parent != nullptr) {
			return m_parent->GetIntersectedBounds().GetIntersected(GetBounds()).value_or(GetBounds());
		}
		return GetBounds();
	}

	float Range::Normalized(float value) {
		const float w = maximum - minimum;
		return (value - minimum) / w;
	}

	float Range::Remap(Range other, float value) {
		const float w = maximum - minimum;
		float n = other.Normalized(value);
		return minimum + n * w;
	}

	float Range::Constrain(float value) {
		return std::max(std::min(value, maximum), minimum);
	}

}
