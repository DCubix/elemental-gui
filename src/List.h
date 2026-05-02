#pragma once

#include "Element.h"
#include "Layout.h"
#include "Scrollbar.h"
#include "Window.h"

namespace gui {
    template <typename T>
    struct ListItem {
        T value;
        std::string label;
    };

    template <typename T>
    class List : public Element {
    public:
        List()
            : Element(),
              m_scrollbar(nullptr) {
            SetLocalBounds({0, 0, 200, 200});
            selectedIndex.SetOnUpdate([this]{ Invalidate(); });
        }

        std::string StyleKey() const override { return "List"; }

        Property<int> selectedIndex{-1};

        void OnDraw(Graphics& g) override {
            auto style = GetStyle();
            const int barsize = 10;

            Size size = GetSize();
            g.StyledRect(0, 0, size.w, size.h, style);

            // Lazy-init the scrollbar
            if (m_scrollbar == nullptr) {
                m_scrollbar = &m_window->template Create<Scrollbar>();
                m_scrollbar->m_parent = this;
                m_scrollbar->direction = Direction::Vertical;
                m_scrollbar->step = 1;
            }

            EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
            EdgeInsets itemPad = EdgeInsets::FromStyle(style["item"]["padding"]);
            int itemHeight = style["item"].value("height", 22);
            int totalContentHeight =
                static_cast<int>(m_items.size()) * (itemHeight + itemPad.GetVertical());

            Rectangle local{0, 0, size.w, size.h};
            Rectangle padB = pad.Apply(local);

            // Determine if scrollbar is needed
            bool needsScrollbar = totalContentHeight > padB.h;
            m_scrollbar->SetEnabled(needsScrollbar);
            m_scrollbar->SetVisible(needsScrollbar);

            int contentWidth = padB.w;
            if (needsScrollbar) {
                contentWidth -= barsize;
                m_scrollbar->SetRange(0, totalContentHeight - padB.h);
                m_scrollbar->SetLocalBounds(Rectangle(size.w - barsize, 0, barsize, size.h));
            } else {
                m_scrollbar->value = 0;
            }

            int scrollOffset = static_cast<int>(m_scrollbar->value());

            g.ClipPushRect(padB.x, padB.y, contentWidth, padB.h);

            // Draw items
            for (size_t i = 0; i < m_items.size(); i++) {
                const auto& item = m_items[i];
                Rectangle itemRect = GetItemRect(static_cast<int>(i), scrollOffset, contentWidth);

                // Skip items outside visible area
                if (itemRect.y + itemRect.h < padB.y || itemRect.y > padB.y + padB.h)
                    continue;

                auto itemStyle = style["item"];

                // Highlight selected item
                if (static_cast<int>(i) == selectedIndex()) {
                    g.StyledRect(
                        itemRect.x,
                        itemRect.y,
                        itemRect.w,
                        itemRect.h,
                        style["itemSelected"]
                    );
                    Json newTextStyle = style["itemSelected"].value("text", style);
                    newTextStyle.update(style);
                    g.StyledTextBegin(newTextStyle);
                    auto sz = g.MeasureText(item.label);
                    g.StyledTextEnd(
                        item.label,
                        itemRect.x + itemPad.left,
                        itemRect.y + (itemRect.h + sz.size.h) / 2
                    );
                } else {
                    Json newTextStyle = itemStyle.value("text", style);
                    newTextStyle.update(style);
                    g.StyledTextBegin(newTextStyle);
                    auto sz = g.MeasureText(item.label);
                    g.StyledTextEnd(
                        item.label,
                        itemRect.x + itemPad.left,
                        itemRect.y + (itemRect.h + sz.size.h) / 2
                    );
                }
            }
            g.ClipPop();

            if (m_scrollbar->IsVisible()) {
                const auto& slb = m_scrollbar->GetLocalBounds();
                g.Save();
                g.Translate(slb.x, slb.y);
                m_scrollbar->OnDraw(g);
                g.Restore();
            }
        }

        void OnScroll(ScrollEvent e) override {
            if (m_scrollbar == nullptr || !m_scrollbar->IsEnabled())
                return;
            const float scrollSpeed = 30.0f;
            float newVal = m_scrollbar->value() - e.scrollY * scrollSpeed;
            newVal = std::max(
                m_scrollbar->GetRange().minimum,
                std::min(newVal, m_scrollbar->GetRange().maximum)
            );
            m_scrollbar->value = newVal;
        }

        void OnMouseDown(MouseEvent e) override {
            if (e.button != MouseButton::Left)
                return;
            Rectangle b = GetLocalIntersectedBounds();
            int scrollOffset = m_scrollbar ? static_cast<int>(m_scrollbar->value()) : 0;
            int contentWidth = GetContentWidth();

            for (size_t i = 0; i < m_items.size(); i++) {
                auto rect = GetItemRect(static_cast<int>(i), scrollOffset, contentWidth);
                if (rect.HasPoint(e.x, e.y) && rect.Intersects(b)) {
                    selectedIndex = static_cast<int>(i);
                    break;
                }
            }
        }

        EventStatus OnEvent(Event* event) override {
            // Let scrollbar handle events first
            if (m_scrollbar != nullptr && m_scrollbar->IsVisible()) {
                if (m_scrollbar->OnEvent(event) == EventStatus::Consumed)
                    return EventStatus::Consumed;
            }

            // Let base Element handle scroll events (calls OnScroll after bounds check)
            if (event->Type() == EventType::Scroll) {
                return Element::OnEvent(event);
            }

            // Let base Element handle mouse events (calls OnMouseDown after bounds check)
            if (event->Type() == EventType::MouseButton) {
                return Element::OnEvent(event);
            }

            return EventStatus::Active;
        }

        void AddItem(const T& data, const std::string& label) { m_items.push_back({data, label}); }

        void RemoveItem(size_t index) {
            if (index < m_items.size()) {
                m_items.erase(m_items.begin() + index);
            }
        }

        const ListItem<T>& GetSelectedItem() const {
            if (selectedIndex() >= 0 && selectedIndex() < static_cast<int>(m_items.size())) {
                return m_items[selectedIndex()];
            }
            throw std::out_of_range("No item selected");
        }

        const std::vector<ListItem<T>>& GetItems() const { return m_items; }

    private:
        std::vector<ListItem<T>> m_items;
        Scrollbar* m_scrollbar;

        int GetContentWidth() const {
            const int barsize = 16;
            Size size = GetSize();
            EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
            Rectangle padB = pad.Apply({0, 0, size.w, size.h});
            EdgeInsets itemPad = EdgeInsets::FromStyle(GetStyle()["item"]["padding"]);
            int itemHeight = GetStyle()["item"].value("height", 22);
            int totalContentHeight =
                static_cast<int>(m_items.size()) * (itemHeight + itemPad.GetVertical());
            bool needsScrollbar = totalContentHeight > padB.h;
            return needsScrollbar ? padB.w - barsize : padB.w;
        }

        Rectangle GetItemRect(int index, int scrollOffset, int contentWidth) const {
            Size size = GetSize();
            EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
            Rectangle padB = pad.Apply({0, 0, size.w, size.h});

            EdgeInsets itemPad = EdgeInsets::FromStyle(GetStyle()["item"]["padding"]);
            int itemHeight = GetStyle()["item"].value("height", 22);

            return {
                padB.x,
                static_cast<int>(
                    padB.y + index * (itemHeight + itemPad.GetVertical()) - scrollOffset
                ),
                contentWidth,
                static_cast<int>(itemHeight + itemPad.GetVertical())
            };
        }
    };
} // namespace gui
