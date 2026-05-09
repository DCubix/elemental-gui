#include "Container.h"

namespace gui {
    void Container::OnDraw(Graphics& g) {
        Rectangle b = GetBounds();
        for (auto&& e : m_children) {
            if (e == nullptr)
                continue;
            if (e == this)
                continue;
            if (!e->IsDirty()) continue;
            if (e->GetBounds().Intersects(b)) {
                const auto& lb = e->GetLocalBounds();
                g.Save();
                g.Translate(lb.x, lb.y);
                e->OnDraw(g);
                g.Restore();
            }
        }
    }

    EventStatus Container::OnEvent(Event* event) {
        // Try children first
        for (auto&& e : m_children) {
            if (e == nullptr)
                continue;
            if (e == this)
                continue;
            if (e->GetBounds().Intersects(GetBounds())) {
                EventStatus status = e->OnEvent(event);
                if (status == EventStatus::Consumed) {
                    return EventStatus::Consumed;
                }
            }
        }
        return Element::OnEvent(event);
    }

    void Container::Add(Element* element) {
        // prevent adding again the same element
        auto it = std::find(m_children.begin(), m_children.end(), element);
        if (it != m_children.end())
            return;

        element->m_parent = this;
        m_children.push_back(element);

        Invalidate();
    }

    void Container::Remove(Element* element) {
        auto it = std::find(m_children.begin(), m_children.end(), element);
        if (it != m_children.end()) {
            (*it)->m_parent = nullptr;
            m_children.erase(it);
            Invalidate();
        }
    }

    // bool Container::IsDirty() {
    //     bool d = Element::IsDirty();
    //     for (auto&& e : m_children) {
    //         if (e == nullptr)
    //             continue;
    //         if (e == this)
    //             continue;
    //         d = d || e->IsDirty();
    //     }
    //     return d;
    // }
} // namespace gui