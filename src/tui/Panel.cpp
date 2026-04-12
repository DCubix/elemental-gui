#include "Panel.h"

#include "Application.h"

namespace tui {

	Panel::Panel() : Element(), m_backgroundVisible(true) {
		m_layout.SetGap(2);
		m_layout.SetPadding(4);
	}

	void Panel::Add(Element *element, Layout::LayoutDirection dir) {
		element->m_parent = this;
		m_layout.Set(element, dir);
		m_children.push_back(element);
	}

	void Panel::OnDraw(Graphics& g) {
		Rectangle b = GetBounds();
		if (m_backgroundVisible)
			g.StyledRect(b.x, b.y, b.w, b.h, GetApp()->GetStyle()["Panel"]);

		m_layout.Apply(b.x, b.y, b.w, b.h);

		Rectangle c = GetIntersectedBounds();
		g.ClipPush(c.x, c.y, c.w, c.h);
		for (auto&& e : m_children) {
			if (e == nullptr) continue;
			if (e == this) continue;
			if (e->GetBounds().Intersects(b)) {
				e->OnDraw(g);
			}
		}
		g.ClipPop();
	}

	bool Panel::IsDirty() {
		bool d = Element::IsDirty();
		for (auto&& e : m_children) {
			if (e == nullptr) continue;
			if (e == this) continue;
			d = d || e->IsDirty();
		}
		return d;
	}

}
