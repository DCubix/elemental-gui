#include "Menu.h"

#include "Layout.h"
#include "Window.h"

#include <vector>

namespace gui {

    Menu::Menu()
        : Element() {
        SetAutoSize(true);
        SetVisible(false);
    }

    void Menu::OnDraw(Graphics& g) {
        if (!m_open)
            return;

        Json style = GetStyle();
        EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
        Size size = GetSize();

        g.StyledRect(0, 0, size.w, size.h, style);

        Rectangle padded{
            (int)pad.left,
            (int)pad.top,
            size.w - (int)pad.GetHorizontal(),
            size.h - (int)pad.GetVertical()
        };
        g.ClipPushRect(padded.x, padded.y, padded.w, padded.h);
        for (auto* el : m_items) {
            if (!el)
                continue;

            MenuItem* item = dynamic_cast<MenuItem*>(el);
            if (item && item->IsSeparator()) {
                Json sepStyle = GetWindow()->GetApp()->GetStyle()["MenuSeparator"];
                auto margins = EdgeInsets::FromStyle(sepStyle["margin"]);
                Rectangle ib = item->GetLocalBounds();

                auto col = Color::FromStyle(sepStyle["color"]);
                g.Color(col.r, col.g, col.b, col.a);
                g.LineWidth(1.0f);

                int lineY = ib.y + margins.top;
                g.Line(ib.x + margins.left, lineY, ib.x + ib.w - margins.right, lineY);
                g.Stroke();
            } else {
                const auto& lb = el->GetLocalBounds();
                g.Save();
                g.Translate(lb.x, lb.y);
                el->OnDraw(g);
                g.Restore();
            }
        }
        g.ClipPop();
    }

    EventStatus Menu::OnEvent(Event* event) {
        if (!m_open)
            return EventStatus::Active;

        // Forward events to open submenu first
        if (HasOpenSubMenu()) {
            if (m_activeSubMenuItem->GetSubMenu()->OnEvent(event) == EventStatus::Consumed)
                return EventStatus::Consumed;
        }

        // Forward events to items
        for (auto* item : m_items) {
            if (item->OnEvent(event) == EventStatus::Consumed &&
                event->Type() != EventType::MouseMotion)
                return EventStatus::Consumed;
        }

        // Dismiss on outside click
        if (event->Type() == EventType::MouseButton) {
            auto* e = dynamic_cast<MouseEvent*>(event);
            if (e->pressed && !HitTest(e->x, e->y)) {
                HideAll();
                return EventStatus::Consumed;
            }
        }

        // Escape: close submenu or self
        if (event->Type() == EventType::Key) {
            auto* e = dynamic_cast<KeyEvent*>(event);
            if (e->pressed && e->key == Key::Escape) {
                if (m_activeSubMenuItem)
                    CloseSubMenu();
                else
                    Hide();
                return EventStatus::Consumed;
            }
        }

        return EventStatus::Active;
    }

    bool Menu::HitTest(int x, int y) const {
        if (GetBounds().HasPoint(x, y))
            return true;
        if (HasOpenSubMenu())
            return m_activeSubMenuItem->GetSubMenu()->HitTest(x, y);
        return false;
    }

    bool Menu::HasOpenSubMenu() const {
        return m_activeSubMenuItem && m_activeSubMenuItem->GetSubMenu() &&
               m_activeSubMenuItem->GetSubMenu()->IsOpen();
    }

    Size Menu::GetPreferredSize() const {
        Json style = GetStyle();
        EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
        int maxW = 0, totalH = 0;
        for (auto* item : m_items) {
            Size s = item->GetPreferredSize();
            maxW = std::max(maxW, s.w);
            totalH += s.h;
        }
        return {maxW + (int)pad.GetHorizontal(), totalH + (int)pad.GetVertical()};
    }

    void Menu::Add(Element* item) {
        item->m_parent = this;
        item->m_window = m_window;
        m_items.push_back(item);
    }

    void Menu::Show(int x, int y) {
        for (auto* item : m_items)
            item->m_window = m_window;
        Size s = GetPreferredSize();
        SetLocalBounds({x, y, s.w, s.h});
        PlaceItems();
        m_open = true;
        SetVisible(true);
        Invalidate();
    }

    void Menu::Hide() {
        CloseSubMenu();
        m_open = false;
        SetVisible(false);
        m_window->DismissPopup(this);
        NotifyListeners();
        std::vector<Element*> toDestroy;
        CollectAll(toDestroy);
        for (auto* el : toDestroy)
            m_window->ScheduleDestroy(el);
    }

    void Menu::HideAll() {
        // Walk up to root menu, then hide from there down
        if (m_parentMenu) {
            m_parentMenu->HideAll();
            return;
        }
        Hide();
    }

    void Menu::OpenSubMenu(MenuItem* item) {
        if (m_activeSubMenuItem == item)
            return;
        CloseSubMenu();

        auto* subMenu = item->GetSubMenu();
        if (!subMenu)
            return;

        m_activeSubMenuItem = item;

        Rectangle itemBounds = item->GetBounds();
        subMenu->m_window = m_window;
        subMenu->SetParentMenu(this);
        m_window->ShowPopup(subMenu);
        subMenu->Show(itemBounds.x + itemBounds.w, itemBounds.y);
    }

    void Menu::CloseSubMenu() {
        if (!m_activeSubMenuItem)
            return;
        auto* subMenu = m_activeSubMenuItem->GetSubMenu();
        if (subMenu && subMenu->IsOpen()) {
            subMenu->m_open = false;
            subMenu->SetVisible(false);
            subMenu->CloseSubMenu();
            m_window->DismissPopup(subMenu);
        }
        // Submenu is now closed — the item's OnMouseLeave kept hover alive while it was open,
        // so we must reset it manually now that the submenu is gone.
        m_activeSubMenuItem->ClearHoverState();
        m_activeSubMenuItem = nullptr;
        Invalidate();
    }

    void Menu::CollectAll(std::vector<Element*>& out) {
        out.push_back(this);
        for (auto* el : m_items) {
            out.push_back(el);

            MenuItem* item = dynamic_cast<MenuItem*>(el);
            if (item) {
                if (auto* sub = item->GetSubMenu())
                    sub->CollectAll(out);
            }
        }
    }

    void Menu::PlaceItems() {
        Json style = GetStyle();
        EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
        Size prefSize = GetPreferredSize();
        int y = (int)pad.top;
        for (auto* item : m_items) {
            Size s = item->GetPreferredSize();
            item->SetPosition({(int)pad.left, y});
            item->SetSize({prefSize.w - (int)pad.GetHorizontal(), s.h});
            y += s.h;
        }
    }

} // namespace gui