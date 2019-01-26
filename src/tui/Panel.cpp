#include "Panel.h"

#include "Application.h"

namespace tui {

	Panel::Panel() : Element() {
		m_layout.gap(2);
		m_layout.padding(4);
	}

	void Panel::add(Element *element, Layout::LayoutDirection dir) {
		if (dir == Layout::LayoutDirection::None) return;
		element->m_parent = this;
		m_layout.set(element, dir);
	}

	void Panel::onDraw(Graphics& g) {
		Rect b = bounds();
		g.styledRect(b.x, b.y, b.w, b.h, app()->style()["Panel"]);

		m_layout.perform(b.x, b.y, b.w, b.h);

		Rect c = intersectedBounds();
		g.clipPush(c.x, c.y, c.w, c.h);
		for (auto&& e : m_layout.elements()) {
			if (e == nullptr) continue;
			if (e == this) continue;
			if (e->bounds().intersects(b)) {
				e->onDraw(g);
			}
		}
		g.clipPop();
	}

	bool Panel::dirty() {
		bool d = Element::dirty();
		for (auto&& e : m_layout.elements()) {
			if (e == nullptr) continue;
			if (e == this) continue;
			d = d || e->dirty();
		}
		return d;
	}

}
