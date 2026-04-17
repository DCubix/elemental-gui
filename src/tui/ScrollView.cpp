#include "ScrollView.h"

#include "Application.h"

namespace tui {

	ScrollView::ScrollView()
		: Element(),
		  m_element(nullptr),
		  m_verticalScrollbar(nullptr),
		  m_horizontalScrollbar(nullptr)
	{}

	EventStatus ScrollView::OnEvent(Event* event) {
		Rectangle b = GetIntersectedBounds();

		if (m_verticalScrollbar != nullptr && m_verticalScrollbar->IsVisible()) {
			if (m_verticalScrollbar->OnEvent(event) == EventStatus::Consumed)
				return EventStatus::Consumed;
		}

		if (m_horizontalScrollbar != nullptr && m_horizontalScrollbar->IsVisible()) {
			if (m_horizontalScrollbar->OnEvent(event) == EventStatus::Consumed)
				return EventStatus::Consumed;
		}

		if (event->Type() == EventType::Scroll) {
			ScrollEvent* e = dynamic_cast<ScrollEvent*>(event);
			Rectangle full = GetIntersectedBounds();
			if (full.HasPoint(e->mouseX, e->mouseY)) {
				const float scrollSpeed = 30.0f;
				bool shift = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;

				if (shift) {
					if (m_horizontalScrollbar != nullptr && m_horizontalScrollbar->IsEnabled()) {
						float newVal = m_horizontalScrollbar->GetValue() - e->scrollY * scrollSpeed;
						newVal = std::max((float)m_horizontalScrollbar->GetRange().minimum,
								 std::min(newVal, (float)m_horizontalScrollbar->GetRange().maximum));
						m_horizontalScrollbar->SetValue(newVal);
						return EventStatus::Consumed;
					}
				} else {
					if (m_verticalScrollbar != nullptr && m_verticalScrollbar->IsEnabled()) {
						float newVal = m_verticalScrollbar->GetValue() - e->scrollY * scrollSpeed;
						newVal = std::max((float)m_verticalScrollbar->GetRange().minimum,
								 std::min(newVal, (float)m_verticalScrollbar->GetRange().maximum));
						m_verticalScrollbar->SetValue(newVal);
						return EventStatus::Consumed;
					}
				}

				// Also handle native horizontal scroll (e.g. trackpad)
				if (e->scrollX != 0.0f && m_horizontalScrollbar != nullptr && m_horizontalScrollbar->IsEnabled()) {
					float newVal = m_horizontalScrollbar->GetValue() - e->scrollX * scrollSpeed;
					newVal = std::max((float)m_horizontalScrollbar->GetRange().minimum,
							 std::min(newVal, (float)m_horizontalScrollbar->GetRange().maximum));
					m_horizontalScrollbar->SetValue(newVal);
					return EventStatus::Consumed;
				}
			}
		}

		if (event->Type() == EventType::MouseButton) {
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			if (b.HasPoint(e->x, e->y) && m_element != nullptr) {
				return m_element->OnEvent(event);
			}
		}

		return EventStatus::Active;
	}

	void ScrollView::OnDraw(Graphics& g) {
		if (m_verticalScrollbar == nullptr) {
			m_verticalScrollbar = &GetApp()->Create<Scrollbar>();
			m_horizontalScrollbar = &GetApp()->Create<Scrollbar>();

			m_verticalScrollbar->m_parent = this;
			m_horizontalScrollbar->m_parent = this;

			m_verticalScrollbar->SetDirection(Direction::Vertical);
			m_horizontalScrollbar->SetDirection(Direction::Horizontal);
			m_verticalScrollbar->SetStep(1);
			m_horizontalScrollbar->SetStep(1);
		}

		m_verticalScrollbar->SetEnabled(false);
		m_horizontalScrollbar->SetEnabled(false);
		m_verticalScrollbar->SetVisible(false);
		m_horizontalScrollbar->SetVisible(false);

		Rectangle b = GetBounds();
		g.StyledRect(b.x, b.y, b.w, b.h, GetStyle()["Panel"]);

		const int barsize = 16;

		Rectangle c = GetIntersectedBounds();

		if (m_element != nullptr) {
			m_element->SetAutoSize(true);
			Size ps = m_element->GetPreferredSize();

			// First pass: determine which scrollbars are needed
			bool needsVertical = m_verticalEnabled && ps.h > b.h;
			bool needsHorizontal = m_horizontalEnabled && ps.w > b.w;

			// Second pass: account for scrollbar space stealing
			if (needsVertical && !needsHorizontal && m_horizontalEnabled) {
				needsHorizontal = ps.w > (b.w - barsize);
			}
			if (needsHorizontal && !needsVertical && m_verticalEnabled) {
				needsVertical = ps.h > (b.h - barsize);
			}

			int contentWidth = needsVertical ? (b.w - barsize) : b.w;
			int contentHeight = needsHorizontal ? (b.h - barsize) : b.h;

			if (needsHorizontal) {
				m_horizontalScrollbar->SetEnabled(true);
				m_horizontalScrollbar->SetVisible(true);
				m_horizontalScrollbar->SetRange(0, ps.w - contentWidth);
				m_horizontalScrollbar->SetLocalBounds(Rectangle(0, b.h - barsize, contentWidth, barsize));
			} else {
				m_horizontalScrollbar->SetValue(0);
			}

			if (needsVertical) {
				m_verticalScrollbar->SetEnabled(true);
				m_verticalScrollbar->SetVisible(true);
				m_verticalScrollbar->SetRange(0, ps.h - contentHeight);
				m_verticalScrollbar->SetLocalBounds(Rectangle(b.w - barsize, 0, barsize, contentHeight));
			} else {
				m_verticalScrollbar->SetValue(0);
			}

			m_element->SetPosition({
				-static_cast<int>(m_horizontalScrollbar->GetValue()),
				-static_cast<int>(m_verticalScrollbar->GetValue())
			});

			if (needsVertical && !needsHorizontal) {
				// Only vertical scrolling: fit element width to content area
				m_element->SetSize({ contentWidth, std::max(ps.h, contentHeight) });
			} else if (needsHorizontal && !needsVertical) {
				// Only horizontal scrolling: fit element height to content area
				m_element->SetSize({ std::max(ps.w, contentWidth), contentHeight });
			} else {
				m_element->SetSize({ std::max(ps.w, contentWidth), std::max(ps.h, contentHeight) });
			}

			g.ClipPush(c.x, c.y, contentWidth, contentHeight);
			m_element->OnDraw(g);
			g.ClipPop();
		}

		if (m_horizontalScrollbar->IsVisible()) m_horizontalScrollbar->OnDraw(g);
		if (m_verticalScrollbar->IsVisible()) m_verticalScrollbar->OnDraw(g);
	}

	void ScrollView::SetElement(Element* e) {
		e->m_parent = this;
		m_element = e;
		Invalidate();
	}

	bool ScrollView::IsDirty() {
		bool d = Element::IsDirty();
		if (m_element != nullptr) {
			d = d || m_element->IsDirty();
		}
		if (m_horizontalScrollbar != nullptr) {
			d = d || m_horizontalScrollbar->IsDirty();
		}
		if (m_verticalScrollbar != nullptr) {
			d = d || m_verticalScrollbar->IsDirty();
		}
		return d;
	}
}
