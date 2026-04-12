#include "ScrollView.h"

#include "Application.h"

namespace tui {

	ScrollView::ScrollView()
		: Element(),
		  m_element(nullptr),
		  m_verticalSlider(nullptr),
		  m_horizontalSlider(nullptr)
	{}

	EventStatus ScrollView::OnEvent(Event* event) {
		const int barsize = 16;
		Rectangle b = GetIntersectedBounds();
		b.w -= barsize;
		b.h -= barsize;

		if (m_verticalSlider != nullptr) {
			if (m_verticalSlider->OnEvent(event) == EventStatus::Consumed)
				return EventStatus::Consumed;
		}

		if (m_horizontalSlider != nullptr) {
			if (m_horizontalSlider->OnEvent(event) == EventStatus::Consumed)
				return EventStatus::Consumed;
		}

		if (event->Type() == EventType::ScrollEventType) {
			ScrollEvent* e = dynamic_cast<ScrollEvent*>(event);
			Rectangle full = GetIntersectedBounds();
			if (full.HasPoint(e->mouseX, e->mouseY)) {
				const float scrollSpeed = 30.0f;
				bool shift = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;

				if (shift) {
					if (m_horizontalSlider != nullptr && m_horizontalSlider->IsEnabled()) {
						float newVal = m_horizontalSlider->GetValue() - e->scrollY * scrollSpeed;
						newVal = std::max((float)m_horizontalSlider->GetRange().minimum,
								 std::min(newVal, (float)m_horizontalSlider->GetRange().maximum));
						m_horizontalSlider->SetValue(newVal);
						return EventStatus::Consumed;
					}
				} else {
					if (m_verticalSlider != nullptr && m_verticalSlider->IsEnabled()) {
						float newVal = m_verticalSlider->GetValue() - e->scrollY * scrollSpeed;
						newVal = std::max((float)m_verticalSlider->GetRange().minimum,
								 std::min(newVal, (float)m_verticalSlider->GetRange().maximum));
						m_verticalSlider->SetValue(newVal);
						return EventStatus::Consumed;
					}
				}

				// Also handle native horizontal scroll (e.g. trackpad)
				if (e->scrollX != 0.0f && m_horizontalSlider != nullptr && m_horizontalSlider->IsEnabled()) {
					float newVal = m_horizontalSlider->GetValue() - e->scrollX * scrollSpeed;
					newVal = std::max((float)m_horizontalSlider->GetRange().minimum,
							 std::min(newVal, (float)m_horizontalSlider->GetRange().maximum));
					m_horizontalSlider->SetValue(newVal);
					return EventStatus::Consumed;
				}
			}
		}

		if (event->Type() == EventType::MouseEventType) {
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			if (b.HasPoint(e->x, e->y) && m_element != nullptr) {
				return m_element->OnEvent(event);
			}
		}

		return EventStatus::Active;
	}

	void ScrollView::OnDraw(Graphics& g) {
		if (m_verticalSlider == nullptr) {
			m_verticalSlider = &GetApp()->Create<Slider>();
			m_horizontalSlider = &GetApp()->Create<Slider>();

			m_verticalSlider->m_parent = this;
			m_horizontalSlider->m_parent = this;

			m_verticalSlider->SetOrientation(Slider::Vertical);
			m_horizontalSlider->SetOrientation(Slider::Horizontal);
			m_verticalSlider->SetStep(1);
			m_horizontalSlider->SetStep(1);
		}

		m_verticalSlider->SetEnabled(false);
		m_horizontalSlider->SetEnabled(false);
		m_verticalSlider->SetVisible(false);
		m_horizontalSlider->SetVisible(false);

		Rectangle b = GetBounds();
		g.StyledRect(b.x, b.y, b.w, b.h, GetApp()->GetStyle()["Panel"]);

		const int barsize = 16;

		Rectangle c = GetIntersectedBounds();

		if (m_element != nullptr) {
			m_element->SetAutoSize(true);

			m_element->GetLocalBounds().x = -m_horizontalSlider->GetValue();
			m_element->GetLocalBounds().y = -m_verticalSlider->GetValue();

			Size ps = m_element->GetPreferredSize();

			int maxx = ps.w;
			int maxy = ps.h;

			int maxregionx = b.w - barsize;
			int maxregiony = b.h - barsize;

			if (maxx > maxregionx) {
				m_horizontalSlider->SetEnabled(true);
				m_horizontalSlider->SetVisible(true);
				m_horizontalSlider->SetRange(0, maxx - maxregionx);
				m_horizontalSlider->SetLocalBounds(Rectangle(0, b.h - barsize, maxregionx, barsize));
			}
			if (maxy > maxregiony) {
				m_verticalSlider->SetEnabled(true);
				m_verticalSlider->SetVisible(true);
				m_verticalSlider->SetRange(0, maxy - maxregiony);
				m_verticalSlider->SetLocalBounds(Rectangle(b.w - barsize, 0, barsize, maxregiony));
			}

			m_element->GetLocalBounds().w = std::max(ps.w, maxregionx);
			m_element->GetLocalBounds().h = std::max(ps.h, maxregiony);

			Rectangle i{ c.x, c.y, maxregionx, maxregiony };
			g.ClipPush(i.x, i.y, i.w, i.h);
			m_element->OnDraw(g);
			g.ClipPop();
		}

		if (m_horizontalSlider != nullptr) {
			if (m_horizontalSlider->IsVisible()) m_horizontalSlider->OnDraw(g);
			if (m_verticalSlider->IsVisible()) m_verticalSlider->OnDraw(g);
		}
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
		if (m_horizontalSlider != nullptr) {
			d = d || m_horizontalSlider->IsDirty();
		}
		if (m_verticalSlider != nullptr) {
			d = d || m_verticalSlider->IsDirty();
		}
		return d;
	}
}
