#include "ScrollView.h"

#include "Application.h"

namespace tui {

	ScrollView::ScrollView()
		: Element(),
		  m_element(nullptr),
		  m_verticalSlider(nullptr),
		  m_horizontalSlider(nullptr)
	{}

	EventStatus ScrollView::onEvent(Event* event) {
		const int barsize = 16;
		Rect b = intersectedBounds();
		b.w -= barsize;
		b.h -= barsize;

		if (m_verticalSlider != nullptr) {
			if (m_verticalSlider->onEvent(event) == EventStatus::Consumed)
				return EventStatus::Consumed;
		}

		if (m_horizontalSlider != nullptr) {
			if (m_horizontalSlider->onEvent(event) == EventStatus::Consumed)
				return EventStatus::Consumed;
		}

		if (event->type() == MouseEventType) {
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			if (b.hasPoint(e->x, e->y) && m_element != nullptr) {
				return m_element->onEvent(event);
			}
		}

		return EventStatus::Active;
	}

	void ScrollView::onDraw(Graphics& g) {
		if (m_verticalSlider == nullptr) {
			m_verticalSlider = app()->create<Slider>();
			m_horizontalSlider = app()->create<Slider>();

			m_verticalSlider->m_parent = this;
			m_horizontalSlider->m_parent = this;

			m_verticalSlider->orientation(Slider::Vertical);
			m_horizontalSlider->orientation(Slider::Horizontal);
			m_verticalSlider->step(1);
			m_horizontalSlider->step(1);
		}

		m_verticalSlider->enabled(false);
		m_horizontalSlider->enabled(false);
		m_verticalSlider->visible(false);
		m_horizontalSlider->visible(false);

		Rect b = bounds();
		g.styledRect(b.x, b.y, b.w, b.h, app()->style()["Panel"]);

		const int barsize = 16;

		Rect c = intersectedBounds();

		if (m_element != nullptr) {
			m_element->autoSize(true);

			m_element->localBounds().x = -m_horizontalSlider->value();
			m_element->localBounds().y = -m_verticalSlider->value();

			Size ps = m_element->preferredSize();

			int maxx = ps.w;
			int maxy = ps.h;

			int maxregionx = b.w - barsize;
			int maxregiony = b.h - barsize;

			if (maxx > maxregionx) {
				m_horizontalSlider->enabled(true);
				m_horizontalSlider->visible(true);
				m_horizontalSlider->range(0, maxx - maxregionx);
				m_horizontalSlider->localBounds(Rect(0, b.h - barsize, maxregionx, barsize));
			}
			if (maxy > maxregiony) {
				m_verticalSlider->enabled(true);
				m_verticalSlider->visible(true);
				m_verticalSlider->range(0, maxy - maxregiony);
				m_verticalSlider->localBounds(Rect(b.w - barsize, 0, barsize, maxregiony));
			}

			m_element->localBounds().w = std::max(ps.w, maxregionx);
			m_element->localBounds().h = std::max(ps.h, maxregiony);

			Rect i{ c.x, c.y, maxregionx, maxregiony };
			g.clipPush(i.x, i.y, i.w, i.h);
			m_element->onDraw(g);
			g.clipPop();
		}

		if (m_horizontalSlider != nullptr) {
			if (m_horizontalSlider->visible()) m_horizontalSlider->onDraw(g);
			if (m_verticalSlider->visible()) m_verticalSlider->onDraw(g);
		}
	}

	void ScrollView::element(Element* e) {
		e->m_parent = this;
		m_element = e;
		invalidate();
	}

	bool ScrollView::dirty() {
		bool d = Element::dirty();
		if (m_element != nullptr) {
			d = d || m_element->dirty();
		}
		if (m_horizontalSlider != nullptr) {
			d = d || m_horizontalSlider->dirty();
		}
		if (m_verticalSlider != nullptr) {
			d = d || m_verticalSlider->dirty();
		}
		return d;
	}
}
