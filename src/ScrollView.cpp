#include "ScrollView.h"

#include "Window.h"

constexpr int SCROLLBAR_SIZE = 12;

namespace gui {

	ScrollView::ScrollView()
		: Element(),
		  m_element(nullptr),
		  m_scrollBar(nullptr),
		  m_scrollDirection(Direction::Vertical)
	{}

	void ScrollView::OnScroll(ScrollEvent e) {
		const float scrollSpeed = 30.0f;
		if (m_scrollBar && m_scrollBar->IsEnabled()) {
			float newVal = m_scrollBar->GetValue() - e.scrollY * scrollSpeed;
			newVal = std::max((float)m_scrollBar->GetRange().minimum,
					std::min(newVal, (float)m_scrollBar->GetRange().maximum));
			m_scrollBar->SetValue(newVal);
		}
	}

	EventStatus ScrollView::OnEvent(Event* event) {
		if (m_scrollBar && m_scrollBar->IsVisible()) {
			if (m_scrollBar->OnEvent(event) == EventStatus::Consumed)
				return EventStatus::Consumed;
		}

		// Let base Element handle scroll and forward other events to child
		if (event->Type() == EventType::Scroll) {
			// Base Element will call OnScroll after bounds check
			return Element::OnEvent(event);
		}

		// Forward events to child
		if (m_element != nullptr) {
			EventStatus status = m_element->OnEvent(event);
			if (status == EventStatus::Consumed)
				return EventStatus::Consumed;
		}

		return EventStatus::Active;
	}

	void ScrollView::OnDraw(Graphics& g) {
		if (!m_scrollBar) {
			m_scrollBar = &m_window->template Create<Scrollbar>();
			m_scrollBar->m_parent = this;
			m_scrollBar->SetDirection(m_scrollDirection);
			m_scrollBar->SetStep(1);
		}

		m_scrollBar->SetEnabled(false);
		m_scrollBar->SetVisible(false);

		Rectangle b = GetBounds();
		g.StyledRect(b.x, b.y, b.w, b.h, GetStyle()["Panel"]);

		Rectangle c = GetIntersectedBounds();

		if (m_element != nullptr) {
			m_element->SetAutoSize(true);
			
			if (m_scrollDirection == Direction::Vertical) {
				SolveScrollVertical();
			} else {
				SolveScrollHorizontal();
			}

			g.ClipPushRect(c.x, c.y, c.w, c.h);
			m_element->OnDraw(g);
			g.ClipPop();
		}

		if (m_scrollBar->IsVisible()) m_scrollBar->OnDraw(g);
	}

	void ScrollView::SetElement(Element* e) {
		e->m_parent = this;
		m_element = e;
		Invalidate();
	}

	void ScrollView::SetScrollDirection(Direction dir) {
		m_scrollDirection = dir;
		if (m_scrollBar) {
			m_scrollBar->SetDirection(dir);
		}
		Invalidate();
	}

	bool ScrollView::IsDirty() {
		bool d = Element::IsDirty();
		if (m_element != nullptr) {
			d = d || m_element->IsDirty();
		}
		if (m_scrollBar != nullptr) {
			d = d || m_scrollBar->IsDirty();
		}
		return d;
	}
    
	void ScrollView::SolveScrollVertical()
    {
		Rectangle b = GetBounds();
		Size ps = m_element->GetPreferredSize();

		int viewSize = ps.h;
		int contentSize = b.h;

		int scrollAxisSize = b.h;
		int crossAxisSize = b.w;

		// Needs scrollbar?
		if (viewSize > contentSize) {
			crossAxisSize -= SCROLLBAR_SIZE;
			m_scrollBar->SetRange(0, ps.h - scrollAxisSize);
			m_scrollBar->SetEnabled(true);
			m_scrollBar->SetVisible(true);
			m_scrollBar->SetLocalBounds(
				Rectangle(b.w - SCROLLBAR_SIZE, 0, SCROLLBAR_SIZE, b.h)
			);
		} else {
			m_scrollBar->SetValue(0);
		}

		m_element->SetLocalBounds({
			0, -static_cast<int>(m_scrollBar->GetValue()),
			crossAxisSize, ps.h
		});
    }
    
	void ScrollView::SolveScrollHorizontal()
    {
		Rectangle b = GetBounds();
		Size ps = m_element->GetPreferredSize();

		int viewSize = ps.w;
		int contentSize = b.w;

		int scrollAxisSize = b.w;
		int crossAxisSize = b.h;

		// Needs scrollbar?
		if (viewSize > contentSize) {
			crossAxisSize -= SCROLLBAR_SIZE;
			m_scrollBar->SetRange(0, ps.w - scrollAxisSize);
			m_scrollBar->SetEnabled(true);
			m_scrollBar->SetVisible(true);
			m_scrollBar->SetLocalBounds(
				Rectangle(0, b.h - SCROLLBAR_SIZE, b.w, SCROLLBAR_SIZE)
			);
		} else {
			m_scrollBar->SetValue(0);
		}

		m_element->SetLocalBounds({
			-static_cast<int>(m_scrollBar->GetValue()), 0,
			ps.w, crossAxisSize
		});
    }
}
