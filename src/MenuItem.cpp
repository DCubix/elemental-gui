#include "MenuItem.h"

#include "Layout.h"
#include "Menu.h"
#include "Window.h"

static constexpr int CHECK_AREA = 20;
static constexpr int ICON_SIZE = 16;
static constexpr int ICON_GAP = 4;
static constexpr int SUBMENU_ARROW_AREA = 16;

namespace gui {

    MenuItem::MenuItem(const std::string& text)
        : Element(),
          m_text(text) {
        SetLocalBounds({0, 0, 100, 20});
        SetAutoSize(true);
    }

    void MenuItem::OnDraw(Graphics& g) {
        if (m_separator)
            return;

        Json style = GetStyle()["MenuItem"];
        EdgeInsets padding = EdgeInsets::FromStyle(style["padding"]);
        Json textStyle = GetStyle()["DefaultText"];
        Size size = GetSize();

        // Background
        Json bgStyle;
        switch (m_state) {
            case ButtonState::Normal:
                bgStyle = style["normal"];
                break;
            case ButtonState::Hover:
                bgStyle = style["hover"];
                break;
            case ButtonState::Click:
                bgStyle = style["click"];
                break;
        }
        g.StyledRect(0, 0, size.w, size.h, bgStyle);

        int cx = (int)padding.left;
        int cy = (int)padding.top;
        int contentH = size.h - (int)padding.GetVertical();

        // Checkmark
        if (m_checked) {
            Json checkmarkStyle = GetStyle()["checkmark"];
            g.DrawSVG(checkmarkStyle, cx + 2, cy, ICON_SIZE, ICON_SIZE);
        }
        cx += CHECK_AREA;

        // Icon
        if (m_icon) {
            g.DrawImage(m_icon, cx, cy + (contentH - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE);
            cx += ICON_SIZE + ICON_GAP;
        }

        // Text
        g.StyledTextBegin(textStyle);
        auto ex = g.MeasureText(m_text);
        g.StyledTextEnd(m_text, cx, cy + (contentH + (int)ex.size.h) / 2);

        // Submenu arrow
        if (m_subMenu) {
            Json arrowStyle = GetStyle()["arrow"];
            int arrowX = size.w - (int)padding.right - ICON_SIZE;
            int arrowY = (size.h - ICON_SIZE) / 2;
            g.DrawSVG(arrowStyle, arrowX, arrowY, ICON_SIZE, ICON_SIZE);
        }
    }

    void MenuItem::OnMouseDown(MouseEvent e) {
        if (m_separator)
            return;
        if (e.button != MouseButton::Left)
            return;

        if (m_state == ButtonState::Hover) {
            if (!m_subMenu) {
                m_state = ButtonState::Click;
                Invalidate();
            }
        }
    }

    void MenuItem::OnMouseUp(MouseEvent e) {
        if (m_separator)
            return;
        if (e.button != MouseButton::Left)
            return;

        if (m_state == ButtonState::Click) {
            Rectangle b = GetBounds();
            if (b.HasPoint(e.x, e.y)) {
                if (m_onClick)
                    m_onClick();
                m_state = ButtonState::Normal;
                Invalidate();
                if (auto* menu = GetParentMenu())
                    menu->HideAll();
            }
        }
    }

    void MenuItem::OnMouseMove(MotionEvent e) {
        if (m_separator)
            return;
        Rectangle b = GetBounds();

        if (b.HasPoint(e.x, e.y)) {
            if (m_state == ButtonState::Normal) {
                m_state = ButtonState::Hover;
                Invalidate();
                NotifyParentMenuHover();
            }
        } else {
            // Don't leave hover if mouse is in our open submenu
            bool inSubMenu = m_subMenu && m_subMenu->IsOpen() && m_subMenu->HitTest(e.x, e.y);
            if (!inSubMenu && m_state != ButtonState::Normal) {
                m_state = ButtonState::Normal;
                Invalidate();
            }
        }
    }

    Menu* MenuItem::GetParentMenu() const {
        return dynamic_cast<Menu*>(GetParent());
    }

    void MenuItem::NotifyParentMenuHover() {
        auto* menu = GetParentMenu();
        if (!menu)
            return;

        if (m_subMenu)
            menu->OpenSubMenu(this);
        else
            menu->CloseSubMenu();
    }

    Size MenuItem::GetPreferredSize() const {
        Json style = GetStyle()["MenuItem"];
        EdgeInsets padding = EdgeInsets::FromStyle(style["padding"]);

        if (IsAutoSize()) {
            if (m_separator)
                return {0, 8};

            auto& g = m_window->GetGraphics();
            const auto textStyle = GetStyle()["DefaultText"];

            g.Save();
            g.StyledTextBegin(textStyle);
            auto sz = g.MeasureText(m_text);
            g.Restore();

            int w = (int)padding.GetHorizontal() + CHECK_AREA + (int)sz.xAdvance;
            int h = (int)padding.GetVertical() + (int)sz.size.h;

            if (m_icon) {
                w += ICON_SIZE + ICON_GAP;
                h = std::max(h, (int)padding.GetVertical() + ICON_SIZE);
            }
            if (m_subMenu) {
                w += SUBMENU_ARROW_AREA;
            }

            return {w, h};
        }
        return Element::GetPreferredSize();
    }

} // namespace gui
