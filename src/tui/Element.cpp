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
		  m_focused(false), m_application(nullptr)
	{}

	void Element::onDraw(Graphics& g) {}

	EventStatus Element::onEvent(Event* event) {
		if (event->type() == MouseEventType) {
			Rect b = intersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			if (b.hasPoint(e->x, e->y) && e->pressed && e->button == 1) {
				requestFocus();
			}
		}
		return EventStatus::Active;
	}

	void Element::invalidate() { m_dirty = true; if (app()) app()->requestRedraw(); }

	void Element::requestFocus() {
		if (app()) {
			app()->focus(this);
			invalidate();
		}
	}

	Rect Element::bounds() const {
		Rect b = m_bounds;
		if (m_parent != nullptr) {
			b.x += m_parent->bounds().x;
			b.y += m_parent->bounds().y;
		}
		return b;
	}

	Rect Element::intersectedBounds() {
		if (m_parent != nullptr) {
			auto&& inter = m_parent->intersectedBounds().getIntersected(m_bounds);
			if (inter.has_value()) {
				return inter.value();
			}
		}
		return m_bounds;
	}

	bool Rect::hasPoint(int x, int y) {
		return x >= this->x &&
			x <= this->x + w &&
			y >= this->y &&
			y <= this->y + h;
	}

	bool Rect::intersects(Rect b) {
		int hwa = w / 2;
		int hwb = b.w / 2;
		int hha = h / 2;
		int hhb = b.h / 2;

		int cax = x + hwa;
		int cay = y + hha;
		int cbx = b.x + hwb;
		int cby = b.y + hhb;

		if (std::abs(cax - cbx) > (hwa + hwb)) return false;
		if (std::abs(cay - cby) > (hha + hhb)) return false;

		return true;
	}

	std::optional<Rect> Rect::getIntersected(Rect b) {
		int xmin = std::max(x, b.x);
		int xmax1 = x + w, xmax2 = b.x + b.w;
		int xmax = std::min(xmax1, xmax2);
		if (xmax > xmin) {
			int ymin = std::max(y, b.y);
			int ymax1 = y + h, ymax2 = b.y + b.h;
			int ymax = std::min(ymax1, ymax2);
			if (ymax > ymin) {
				return Rect(xmin, ymin, xmax - xmin, ymax - ymin);
			}
		}
		return {};
	}

	Layout::Layout()
	{
		for (int i = 0; i < LayoutDirectionCount; i++) m_elements[i] = nullptr;
	}

	void Layout::set(Element* element, Layout::LayoutDirection dir) {
		if (dir == LayoutDirection::None) return;
		m_elements[dir] = element;
	}

	void Layout::perform(int x, int y, int w, int h) {
#define EL(x) m_elements[x]
		int top = m_padding;
		int bottom = h - m_padding;
		int left = m_padding;
		int right = w - m_padding;

		if (EL(Layout::Top) != nullptr) {
			int h = EL(Layout::Top)->bounds().h;
			Size s = calcElementSize(EL(Layout::Top));
			EL(Layout::Top)->localBounds() = Rect(left, top, right - left, h);
			top += s.h + m_gap;
		}
		if (EL(Layout::Bottom) != nullptr) {
			int h = EL(Layout::Bottom)->bounds().h;
			Size s = calcElementSize(EL(Layout::Bottom));
			EL(Layout::Bottom)->localBounds() = Rect(left, bottom - s.h, right - left, h);
			bottom -= s.h + m_gap;
		}
		if (EL(Layout::Right) != nullptr) {
			int w = EL(Layout::Right)->bounds().w;
			Size s = calcElementSize(EL(Layout::Right));
			EL(Layout::Right)->localBounds() = Rect(right - s.w, top, w, bottom - top);
			right -= s.w + m_gap;
		}
		if (EL(Layout::Left) != nullptr) {
			int w = EL(Layout::Left)->bounds().w;
			Size s = calcElementSize(EL(Layout::Left));
			EL(Layout::Left)->localBounds() = Rect(left, top, w, bottom - top);
			left += s.w + m_gap;
		}
		if (EL(Layout::Center) != nullptr) {
			EL(Layout::Center)->localBounds() = Rect(left, top, right - left, bottom - top);
		}
	}

	Size Layout::calcElementSize(Element* element) {
		if (!element->visible()) {
			return { 0, 0 };
		}
		return { element->bounds().w, element->bounds().h };
	}

	float Range::normalized(float value) {
		const float w = maximum - minimum;
		return (value - minimum) / w;
	}

	float Range::remap(Range other, float value) {
		const float w = maximum - minimum;
		float n = other.normalized(value);
		return minimum + n * w;
	}

	float Range::constrain(float value) {
		return std::max(std::min(value, maximum), minimum);
	}

}
