#include "ScrollView.h"

#include "Window.h"

constexpr int SCROLLBAR_SIZE = 12;

namespace gui {

    ScrollView::ScrollView()
        : Element(),
          m_element(nullptr),
          m_scrollBar(nullptr),
          m_scrollDirection(Direction::Vertical) {}

    void ScrollView::OnScroll(ScrollEvent e) {
        const float scrollSpeed = 30.0f;
        if (m_scrollBar && m_scrollBar->IsEnabled()) {
            float newVal = m_scrollBar->value() - e.scrollY * scrollSpeed;
            newVal = std::max(
                (float)m_scrollBar->GetRange().minimum,
                std::min(newVal, (float)m_scrollBar->GetRange().maximum)
            );
            m_scrollBar->value = newVal;
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
            m_scrollBar->direction = m_scrollDirection;
            m_scrollBar->step = 1;
        }

        m_scrollBar->SetEnabled(false);
        m_scrollBar->SetVisible(false);

        Size size = GetSize();
        g.StyledRect(0, 0, size.w, size.h, GetStyle()["Panel"]);

        Rectangle clip = GetLocalIntersectedBounds();

        if (m_element != nullptr) {
            m_element->SetAutoSize(true);

            if (m_scrollDirection == Direction::Vertical) {
                SolveScrollVertical();
            } else {
                SolveScrollHorizontal();
            }

            g.ClipPushRect(clip.x, clip.y, clip.w, clip.h);
            const auto& elb = m_element->GetLocalBounds();
            g.Save();
            g.Translate(elb.x, elb.y);
            m_element->OnDraw(g);
            g.Restore();
            g.ClipPop();
        }

        if (m_scrollBar->IsVisible()) {
            const auto& slb = m_scrollBar->GetLocalBounds();
            g.Save();
            g.Translate(slb.x, slb.y);
            m_scrollBar->OnDraw(g);
            g.Restore();
        }
    }

    void ScrollView::SetElement(Element* e) {
        e->m_parent = this;
        m_element = e;
        Invalidate();
    }

    void ScrollView::SetScrollDirection(Direction dir) {
        m_scrollDirection = dir;
        if (m_scrollBar) {
            m_scrollBar->direction = dir;
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

    void ScrollView::SolveScrollVertical() {
        Size size = GetSize();
        Size ps = m_element->GetPreferredSize();

        int scrollAxisSize = size.h;
        int crossAxisSize = size.w;

        if (ps.h > size.h) {
            crossAxisSize -= SCROLLBAR_SIZE;
            m_scrollBar->SetRange(0, ps.h - scrollAxisSize);
            m_scrollBar->SetEnabled(true);
            m_scrollBar->SetVisible(true);
            m_scrollBar->SetLocalBounds(
                Rectangle(size.w - SCROLLBAR_SIZE, 0, SCROLLBAR_SIZE, size.h)
            );
        } else {
            m_scrollBar->value = 0;
        }

        m_element->SetLocalBounds(
            {0, -static_cast<int>(m_scrollBar->value()), crossAxisSize, ps.h}
        );
    }

    void ScrollView::SolveScrollHorizontal() {
        Size size = GetSize();
        Size ps = m_element->GetPreferredSize();

        int scrollAxisSize = size.w;
        int crossAxisSize = size.h;

        if (ps.w > size.w) {
            crossAxisSize -= SCROLLBAR_SIZE;
            m_scrollBar->SetRange(0, ps.w - scrollAxisSize);
            m_scrollBar->SetEnabled(true);
            m_scrollBar->SetVisible(true);
            m_scrollBar->SetLocalBounds(
                Rectangle(0, size.h - SCROLLBAR_SIZE, size.w, SCROLLBAR_SIZE)
            );
        } else {
            m_scrollBar->value = 0;
        }

        m_element->SetLocalBounds(
            {-static_cast<int>(m_scrollBar->value()), 0, ps.w, crossAxisSize}
        );
    }
} // namespace gui
