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

        const std::vector<Element*>& GetChildren() const { return m_children; }

        template <DerivedFromElement E>
        E* FindChildByTag(const std::string& tag) {
            for (auto el : m_children) {
                if (el->GetTag() == tag) {
                    return dynamic_cast<E*>(el);
                }
            }
            return nullptr;
        }

        template <DerivedFromElement E>
        E* FindChild(std::function<bool(E*)> predicate = nullptr) {
            for (auto el : m_children) {
                E* casted = dynamic_cast<E*>(el);
                if (casted && (predicate == nullptr || predicate(casted))) {
                    return casted;
                }
            }
            return nullptr;
        }

    protected:
        std::vector<Element*> m_children;
    };
} // namespace gui