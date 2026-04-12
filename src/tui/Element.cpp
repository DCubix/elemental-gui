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

	Layout::Layout()
	{
		for (int i = 0; i < LayoutDirectionCount; i++) m_elements[i] = nullptr;
	}

	void Layout::Set(Element* element, Layout::LayoutDirection dir) {
		if (dir == LayoutDirection::None) return;
		m_elements[dir] = element;
	}

	void Layout::Apply(int x, int y, int w, int h) {
#define EL(x) m_elements[x]
		int top = m_padding;
		int bottom = h - m_padding;
		int left = m_padding;
		int right = w - m_padding;

		if (EL(Layout::Top) != nullptr) {
			int h = EL(Layout::Top)->GetBounds().h;
			Size s = CalcElementSize(EL(Layout::Top));
			EL(Layout::Top)->GetLocalBounds() = Rectangle(left, top, right - left, h);
			top += s.h + m_gap;
		}
		if (EL(Layout::Bottom) != nullptr) {
			int h = EL(Layout::Bottom)->GetBounds().h;
			Size s = CalcElementSize(EL(Layout::Bottom));
			EL(Layout::Bottom)->GetLocalBounds() = Rectangle(left, bottom - s.h, right - left, h);
			bottom -= s.h + m_gap;
		}
		if (EL(Layout::Right) != nullptr) {
			int w = EL(Layout::Right)->GetBounds().w;
			Size s = CalcElementSize(EL(Layout::Right));
			EL(Layout::Right)->GetLocalBounds() = Rectangle(right - s.w, top, w, bottom - top);
			right -= s.w + m_gap;
		}
		if (EL(Layout::Left) != nullptr) {
			int w = EL(Layout::Left)->GetBounds().w;
			Size s = CalcElementSize(EL(Layout::Left));
			EL(Layout::Left)->GetLocalBounds() = Rectangle(left, top, w, bottom - top);
			left += s.w + m_gap;
		}
		if (EL(Layout::Center) != nullptr) {
			EL(Layout::Center)->GetLocalBounds() = Rectangle(left, top, right - left, bottom - top);
		}
	}

	Size Layout::CalcElementSize(Element* element) {
		if (!element->IsVisible()) {
			return { 0, 0 };
		}
		return { element->GetBounds().w, element->GetBounds().h };
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
