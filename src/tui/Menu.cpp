#include "Menu.h"
#include "Application.h"
#include "Layout.h"

namespace tui {

    Menu::Menu() : Element() {
        SetAutoSize(true);
        SetVisible(false);
    }

    void Menu::OnDraw(Graphics &g) {
        if (!m_open) return;

        Json style = GetStyle()["Menu"];
        EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
        Rectangle b = GetBounds();

        g.StyledRect(b.x, b.y, b.w, b.h, style);
        PlaceItems();

        Rectangle padded{
            b.x + (int)pad.left,
            b.y + (int)pad.top,
            b.w - (int)pad.GetHorizontal(),
            b.h - (int)pad.GetVertical()
        };
        g.ClipPush(padded.x, padded.y, padded.w, padded.h);
        for (auto* item : m_items) {
            if (item == nullptr) continue;
            if (item->IsSeparator()) {
                Json sepStyle = GetStyle()["MenuSeparator"];
                float mx = sepStyle.value("margin", Json::object()).value("horizontal", 8.0f);
                Rectangle ib = item->GetBounds();
                auto col = sepStyle.value("color", std::vector<float>{0.28f, 0.28f, 0.30f, 1.0f});
                g.Color(col[0], col[1], col[2], col[3]);
                g.LineWidth(1.0f);
                int lineY = ib.y + ib.h / 2;
                g.Line(ib.x + (int)mx, lineY, ib.x + ib.w - (int)mx, lineY);
                g.Stroke();
            } else {
                item->OnDraw(g);
            }
        }
        g.ClipPop();

        // Draw open submenu on top
        if (HasOpenSubMenu()) {
            m_activeSubMenuItem->GetSubMenu()->OnDraw(g);
        }
    }

    EventStatus Menu::OnEvent(Event *event) {
        if (!m_open) return EventStatus::Active;

        // Forward events to open submenu first
        if (HasOpenSubMenu()) {
            if (m_activeSubMenuItem->GetSubMenu()->OnEvent(event) == EventStatus::Consumed)
                return EventStatus::Consumed;
        }

        // Forward events to items
        for (auto* item : m_items) {
            if (item->OnEvent(event) == EventStatus::Consumed)
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
            if (e->pressed && e->key == SDLK_ESCAPE) {
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
        if (GetBounds().HasPoint(x, y)) return true;
        if (HasOpenSubMenu())
            return m_activeSubMenuItem->GetSubMenu()->HitTest(x, y);
        return false;
    }

    bool Menu::HasOpenSubMenu() const {
        return m_activeSubMenuItem
            && m_activeSubMenuItem->GetSubMenu()
            && m_activeSubMenuItem->GetSubMenu()->IsOpen();
    }

    Size Menu::GetPreferredSize() const {
        if (IsAutoSize()) {
            Json style = GetStyle()["Menu"];
            EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
            int maxW = 0, totalH = 0;
            for (auto* item : m_items) {
                Size s = item->GetPreferredSize();
                maxW = std::max(maxW, s.w);
                totalH += s.h;
            }
            return { maxW + (int)pad.GetHorizontal(), totalH + (int)pad.GetVertical() };
        }
        return Element::GetPreferredSize();
    }

    void Menu::Add(MenuItem *item) {
        item->m_parent = this;
        item->m_application = m_application;
        m_items.push_back(item);
    }

    void Menu::Show(int x, int y) {
        for (auto* item : m_items)
            item->m_application = m_application;
        Size s = GetPreferredSize();
        SetLocalBounds({x, y, s.w, s.h});
        m_open = true;
        SetVisible(true);
        Invalidate();
    }

    void Menu::Hide() {
        CloseSubMenu();
        m_open = false;
        SetVisible(false);
        if (m_onDismiss) m_onDismiss();
        Invalidate();
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
        if (m_activeSubMenuItem == item) return;
        CloseSubMenu();

        auto* subMenu = item->GetSubMenu();
        if (!subMenu) return;

        m_activeSubMenuItem = item;

        Rectangle itemBounds = item->GetBounds();
        subMenu->m_application = m_application;
        subMenu->SetParentMenu(this);
        m_application->ShowPopup(subMenu);
        subMenu->Show(itemBounds.x + itemBounds.w, itemBounds.y);
    }

    void Menu::CloseSubMenu() {
        if (!m_activeSubMenuItem) return;
        auto* subMenu = m_activeSubMenuItem->GetSubMenu();
        if (subMenu && subMenu->IsOpen()) {
            subMenu->CloseSubMenu();
            subMenu->m_open = false;
            subMenu->SetVisible(false);
            m_application->DismissPopup(subMenu);
        }
        m_activeSubMenuItem = nullptr;
        Invalidate();
    }

    void Menu::PlaceItems() {
        Json style = GetStyle()["Menu"];
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

}