#include "Menu.h"

#include "Application.h"
#include "FlexLayout.h"

namespace tui {
    Menu::Menu() : Element()
    {
        SetAutoSize(true);
        SetVisible(false);
    }

    void Menu::OnDraw(Graphics &g)
    {
        if (!m_open) return;

        Json style = GetStyle()["Menu"];
        EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);

        Rectangle b = GetBounds();

        // Draw menu background/border
        g.StyledRect(b.x, b.y, b.w, b.h, style);

        PlaceItems();

		Rectangle padded{
			b.x + (int)pad.left,
			b.y + (int)pad.top,
			b.w - (int)pad.GetHorizontal(),
			b.h - (int)pad.GetVertical()
		};
		g.ClipPush(padded.x, padded.y, padded.w, padded.h);
		for (auto&& item : m_items) {
			if (item == nullptr) continue;
			if (item->IsSeparator()) {
				// Draw separator line
				Json sepStyle = GetStyle()["MenuSeparator"];
				float mx = sepStyle.value("margin", Json::object()).value("horizontal", 8.0f);
				float my = sepStyle.value("margin", Json::object()).value("vertical", 4.0f);
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

        // Draw open submenu
        if (m_activeSubMenuItem && m_activeSubMenuItem->GetSubMenu()) {
            auto* subMenu = m_activeSubMenuItem->GetSubMenu();
            if (subMenu->IsOpen()) {
                subMenu->OnDraw(g);
            }
        }
    }

    EventStatus Menu::OnEvent(Event *event)
    {
        if (!m_open) return EventStatus::Active;

        // Forward events to open submenu first
        if (m_activeSubMenuItem && m_activeSubMenuItem->GetSubMenu()) {
            auto* subMenu = m_activeSubMenuItem->GetSubMenu();
            if (subMenu->IsOpen()) {
                EventStatus subStatus = subMenu->OnEvent(event);
                if (subStatus == EventStatus::Consumed)
                    return EventStatus::Consumed;
            }
        }

        // Forward mouse events to items
        for (auto* item : m_items) {
            if (item->OnEvent(event) == EventStatus::Consumed)
                return EventStatus::Consumed;
        }

        // Dismiss on outside click
        if (event->Type() == EventType::MouseButton) {
            MouseEvent* e = dynamic_cast<MouseEvent*>(event);
            Rectangle b = GetBounds();
            bool inSubMenu = false;
            if (m_activeSubMenuItem && m_activeSubMenuItem->GetSubMenu()) {
                auto* subMenu = m_activeSubMenuItem->GetSubMenu();
                if (subMenu->IsOpen()) {
                    inSubMenu = subMenu->GetBounds().HasPoint(e->x, e->y);
                }
            }
            if (!b.HasPoint(e->x, e->y) && !inSubMenu && e->pressed) {
                HideAll();
                return EventStatus::Consumed;
            }
        }

        // Dismiss on Escape
        if (event->Type() == EventType::Key) {
            KeyEvent* e = dynamic_cast<KeyEvent*>(event);
            if (e->pressed && e->key == SDLK_ESCAPE) {
                if (m_activeSubMenuItem) {
                    CloseSubMenu();
                } else {
                    Hide();
                }
                return EventStatus::Consumed;
            }
        }

        return EventStatus::Active;
    }

    Size Menu::GetPreferredSize() const
    {
        if (IsAutoSize()) {
            Json style = GetStyle()["Menu"];
            EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
            int maxW = 0, totalH = 0;
            for (auto item : m_items) {
                Size s = item->GetPreferredSize();
                maxW = std::max(maxW, s.w);
                totalH += s.h;
            }
            return { maxW + (int)pad.GetHorizontal(), totalH + (int)pad.GetVertical() };
        }
        return Element::GetPreferredSize();
    }

    void Menu::Add(MenuItem *item)
    {
        item->m_parent = this;
        item->m_application = m_application;
        m_items.push_back(item);
    }

    void Menu::Show(int x, int y)
    {
        // Propagate application pointer to items
        for (auto* item : m_items) {
            item->m_application = m_application;
        }
        Size s = GetPreferredSize();
        SetLocalBounds({x, y, s.w, s.h});
        m_open = true;
        SetVisible(true);
        Invalidate();
    }

    void Menu::Hide()
    {
        CloseSubMenu();
        m_open = false;
        SetVisible(false);
        if (m_onDismiss) m_onDismiss();
        Invalidate();
    }

    void Menu::HideAll()
    {
        // Close any open submenu chain, then hide self
        CloseSubMenu();
        m_open = false;
        SetVisible(false);
        if (m_onDismiss) m_onDismiss();
        Invalidate();
    }

    void Menu::OpenSubMenu(MenuItem* item)
    {
        if (m_activeSubMenuItem == item) return;
        CloseSubMenu();

        auto* subMenu = item->GetSubMenu();
        if (!subMenu) return;

        m_activeSubMenuItem = item;

        // Position submenu to the right of the item
        // GetBounds() already returns absolute screen coordinates
        Rectangle itemBounds = item->GetBounds();
        int sx = itemBounds.x + itemBounds.w;
        int sy = itemBounds.y;

        subMenu->m_application = m_application;
        subMenu->SetParentMenu(this);
        m_application->ShowPopup(subMenu);
        subMenu->Show(sx, sy);
    }

    void Menu::CloseSubMenu()
    {
        if (!m_activeSubMenuItem) return;
        auto* subMenu = m_activeSubMenuItem->GetSubMenu();
        if (subMenu && subMenu->IsOpen()) {
            subMenu->CloseSubMenu(); // recurse into nested submenus
            subMenu->m_open = false;
            subMenu->SetVisible(false);
            m_application->DismissPopup(subMenu);
        }
        m_activeSubMenuItem = nullptr;
        Invalidate();
    }

    void Menu::PlaceItems()
    {
        Json style = GetStyle()["Menu"];
        EdgeInsets pad = EdgeInsets::FromStyle(style["padding"]);
        Size prefSize = GetPreferredSize();
        int y = (int)pad.top;
        for (auto item : m_items) {
            Size s = item->GetPreferredSize();
            item->SetPosition({(int)pad.left, y});
            item->SetSize({prefSize.w - (int)pad.GetHorizontal(), s.h});
            y += s.h;
        }
    }
}