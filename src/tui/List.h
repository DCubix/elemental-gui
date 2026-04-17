#pragma once

#include "Element.h"
#include "Layout.h"
#include "Scrollbar.h"
#include "Application.h"

namespace tui {
    template<typename T>
    struct ListItem {
        T value;
        std::string label;
    };

    template <typename T>
    class List : public Element {
    public:
        List() : Element(), m_scrollbar(nullptr) {
            SetLocalBounds({0, 0, 200, 200});
        }

        void OnDraw(Graphics& g) override {
            auto style = GetStyle()["List"];
            auto textStyle = GetStyle()["DefaultText"];
            const int barsize = 12;

            Rectangle b = GetBounds();

            // Draw background
            g.StyledRect(b.x, b.y, b.w, b.h, style);

            // Lazy-init the scrollbar
            if (m_scrollbar == nullptr) {
                m_scrollbar = &GetApp()->Create<Scrollbar>();
                m_scrollbar->m_parent = this;
                m_scrollbar->SetDirection(Direction::Vertical);
                m_scrollbar->SetStep(1);
            }

            EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
            EdgeInsets itemPad = EdgeInsets::FromStyle(style["item"]["padding"]);
            int itemHeight = style["item"].value("height", 22);
            int totalContentHeight = static_cast<int>(m_items.size()) * (itemHeight + itemPad.GetVertical());

            Rectangle padB = pad.Apply(b);

            // Determine if scrollbar is needed
            bool needsScrollbar = totalContentHeight > padB.h;
            m_scrollbar->SetEnabled(needsScrollbar);
            m_scrollbar->SetVisible(needsScrollbar);

            int contentWidth = padB.w;
            if (needsScrollbar) {
                contentWidth -= barsize;
                m_scrollbar->SetRange(0, totalContentHeight - padB.h);
                m_scrollbar->SetLocalBounds(Rectangle(b.w - barsize, 0, barsize, b.h));
            } else {
                m_scrollbar->SetValue(0);
            }

            int scrollOffset = static_cast<int>(m_scrollbar->GetValue());

            g.ClipPush(padB.x, padB.y, contentWidth, padB.h);

            // Draw items
            for (size_t i = 0; i < m_items.size(); i++) {
                const auto& item = m_items[i];
                Rectangle itemRect = GetItemRect(static_cast<int>(i), scrollOffset, contentWidth);

                // Skip items outside visible area
                if (itemRect.y + itemRect.h < padB.y || itemRect.y > padB.y + padB.h) continue;

                auto itemStyle = style["item"];

                // Highlight selected item
                if (static_cast<int>(i) == m_selectedIndex) {
                    g.StyledRect(
                        itemRect.x,
                        itemRect.y,
                        itemRect.w,
                        itemRect.h,
                        style["itemSelected"]
                    );
                    Json newTextStyle = style["itemSelected"].value("text", textStyle);
                    newTextStyle.update(textStyle);
                    g.StyledTextBegin(newTextStyle);
                    auto sz = g.MeasureText(item.label);
                    g.StyledTextEnd(
                        item.label, itemRect.x + itemPad.left, itemRect.y + (itemRect.h + sz.height) / 2
                    );
                } else {
                    Json newTextStyle = itemStyle.value("text", textStyle);
                    newTextStyle.update(textStyle);
                    g.StyledTextBegin(newTextStyle);
                    auto sz = g.MeasureText(item.label);
                    g.StyledTextEnd(
                        item.label, itemRect.x + itemPad.left, itemRect.y + (itemRect.h + sz.height) / 2
                    );
                }
            }
            g.ClipPop();

            if (m_scrollbar->IsVisible()) {
                m_scrollbar->OnDraw(g);
            }
        }

        EventStatus OnEvent(Event *event) override {
            // Let scrollbar handle events first
            if (m_scrollbar != nullptr && m_scrollbar->IsVisible()) {
                if (m_scrollbar->OnEvent(event) == EventStatus::Consumed)
                    return EventStatus::Consumed;
            }

            // Handle scroll wheel on the list area
            if (event->Type() == EventType::Scroll && m_scrollbar != nullptr && m_scrollbar->IsEnabled()) {
                ScrollEvent* e = dynamic_cast<ScrollEvent*>(event);
                Rectangle b = GetIntersectedBounds();
                if (b.HasPoint(e->mouseX, e->mouseY)) {
                    const float scrollSpeed = 30.0f;
                    float newVal = m_scrollbar->GetValue() - e->scrollY * scrollSpeed;
                    newVal = std::max(m_scrollbar->GetRange().minimum,
                             std::min(newVal, m_scrollbar->GetRange().maximum));
                    m_scrollbar->SetValue(newVal);
                    return EventStatus::Consumed;
                }
            }

            if (event->Type() == EventType::MouseButton) {
                auto me = static_cast<MouseEvent*>(event);
                Rectangle b = GetBounds();
                int scrollOffset = m_scrollbar ? static_cast<int>(m_scrollbar->GetValue()) : 0;
                int contentWidth = GetContentWidth();

                if (me->button == 1 && me->pressed) {
                    for (size_t i = 0; i < m_items.size(); i++) {
                        auto rect = GetItemRect(static_cast<int>(i), scrollOffset, contentWidth);
                        if (rect.HasPoint(me->x, me->y) && rect.Intersects(b)) {
                            SetSelectedIndex(static_cast<int>(i));
                            return EventStatus::Consumed;
                        }
                    }
                }
            }
            return EventStatus::Active;
        }

        void AddItem(const T& data, const std::string& label) {
            m_items.push_back({data, label});
        }

        void RemoveItem(size_t index) {
            if (index < m_items.size()) {
                m_items.erase(m_items.begin() + index);
            }
        }

        void SetSelectedIndex(int index) {
            if (index < static_cast<int>(m_items.size())) {
                if (m_selectedIndex != index) {
                    m_selectedIndex = index;
                    Invalidate();
                    if (m_onSelectionChanged) {
                        m_onSelectionChanged(m_selectedIndex);
                    }
                }
            }
        }
        int GetSelectedIndex() const { return m_selectedIndex; }

        const ListItem<T>& GetSelectedItem() const {
            if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
                return m_items[m_selectedIndex];
            }
            throw std::out_of_range("No item selected");
        }

        void SetOnSelectionChanged(const ValueChanged<int>& callback) {
            m_onSelectionChanged = callback;
        }

        const std::vector<ListItem<T>>& GetItems() const { return m_items; }

    private:
        int m_selectedIndex{ -1 };
        std::vector<ListItem<T>> m_items;
        ValueChanged<int> m_onSelectionChanged;
        Scrollbar* m_scrollbar;

        int GetContentWidth() const {
            const int barsize = 16;
            Rectangle b = GetBounds();
            EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["List"]["padding"]);
            Rectangle padB = pad.Apply(b);
            EdgeInsets itemPad = EdgeInsets::FromStyle(GetStyle()["List"]["item"]["padding"]);
            int itemHeight = GetStyle()["List"]["item"].value("height", 22);
            int totalContentHeight = static_cast<int>(m_items.size()) * (itemHeight + itemPad.GetVertical());
            bool needsScrollbar = totalContentHeight > padB.h;
            return needsScrollbar ? padB.w - barsize : padB.w;
        }

        Rectangle GetItemRect(int index, int scrollOffset, int contentWidth) const {
            Rectangle b = GetBounds();
            EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["List"]["padding"]);
            Rectangle padB = pad.Apply(b);

            EdgeInsets itemPad = EdgeInsets::FromStyle(GetStyle()["List"]["item"]["padding"]);
            int itemHeight = GetStyle()["List"]["item"].value("height", 22);

            return {
                padB.x,
                padB.y + index * (itemHeight + itemPad.GetVertical()) - scrollOffset,
                contentWidth,
                itemHeight + itemPad.GetVertical()
            };
        }
    };
}