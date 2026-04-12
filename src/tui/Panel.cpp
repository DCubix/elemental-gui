#include "Panel.h"
#include "FlexLayout.h"

#include "Application.h"

namespace tui {

	Panel::Panel() : Element(), m_backgroundVisible(true) {
		m_layout = std::make_unique<FlexLayout>();
		m_layout->SetGap(4);
		m_layout->SetPadding(8);
	}

	void Panel::Add(Element *element) {
		// prevent adding again the same element
		for (auto&& e : m_children) {
			if (e == element) return;
		}

		element->m_parent = this;
		m_children.push_back(element);
		if (FlexLayout* flex = dynamic_cast<FlexLayout*>(m_layout.get())) {
			flex->Add(element);
		}
	}

	void Panel::SetLayout(std::unique_ptr<Layout> layout) {
		m_layout = std::move(layout);
	}

	void Panel::OnDraw(Graphics& g) {
		Rectangle b = GetBounds();
		if (m_backgroundVisible)
			g.StyledRect(b.x, b.y, b.w, b.h, GetStyle()["Panel"]);

		if (m_layout)
			m_layout->Apply(b.x, b.y, b.w, b.h);

		int pad = m_layout ? m_layout->GetPadding() : 0;
		Rectangle c = GetIntersectedBounds();
		Rectangle padded = { c.x + pad, c.y + pad, c.w - 2 * pad, c.h - 2 * pad };
		g.ClipPush(padded.x, padded.y, padded.w, padded.h);
		for (auto&& e : m_children) {
			if (e == nullptr) continue;
			if (e == this) continue;
			if (e->GetBounds().Intersects(b)) {
				e->OnDraw(g);
			}
		}
		g.ClipPop();
	}

	Size Panel::GetPreferredSize() {
		if (!IsAutoSize()) return Element::GetPreferredSize();

		int pad = m_layout ? m_layout->GetPadding() : 0;
		int gap = m_layout ? m_layout->GetGap() : 0;
		FlexDirection dir = FlexDirection::Row;
		if (FlexLayout* flex = dynamic_cast<FlexLayout*>(m_layout.get())) {
			dir = flex->GetDirection();
		}

		int mainTotal = 0;
		int crossMax = 0;
		int visibleCount = 0;

		for (auto& e : m_children) {
			if (!e || e == this || !e->IsVisible()) continue;
			Size s = e->GetPreferredSize();
			int ms = (dir == FlexDirection::Row) ? s.w : s.h;
			int cs = (dir == FlexDirection::Row) ? s.h : s.w;
			mainTotal += ms;
			if (cs > crossMax) crossMax = cs;
			visibleCount++;
		}

		if (visibleCount > 1) mainTotal += (visibleCount - 1) * gap;
		mainTotal += 2 * pad;
		crossMax += 2 * pad;

		if (dir == FlexDirection::Row)
			return { mainTotal, crossMax };
		else
			return { crossMax, mainTotal };
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
