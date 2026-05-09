#pragma once

#include "Element.h"

namespace gui {
    class Container : public Element {
    public:
        Container() = default;
        ~Container() = default;

        virtual void OnDraw(Graphics& g) override;
        virtual EventStatus OnEvent(Event* event) override;

        virtual Size GetPreferredSize() const override = 0;

        virtual void Add(Element* element);
        virtual void Remove(Element* element);

        const std::vector<Element*>& GetChildren() { return m_children; }

    protected:
        std::vector<Element*> m_children;

        // virtual bool IsDirty() override;
    };
} // namespace gui