#include "MenuItem.h"
#include "Menu.h"

#include "Layout.h"
#include "Application.h"

static constexpr int CHECK_AREA = 20;
static constexpr int ICON_SIZE = 16;
static constexpr int ICON_GAP = 4;
static constexpr int SUBMENU_ARROW_AREA = 16;

namespace tui {

    MenuItem::MenuItem(const std::string &text)
        : Element(), m_text(text) {
        SetLocalBounds({0, 0, 100, 20});
        SetAutoSize(true);
    }

    void MenuItem::OnDraw(Graphics &g) {
        if (m_separator) return;

        Json style = GetStyle()["MenuItem"];
        EdgeInsets padding = EdgeInsets::FromStyle(style["padding"]);
        Json textStyle = GetStyle()["DefaultText"];
        Rectangle b = GetBounds();

        // Background
        Json bgStyle;
        switch (m_state) {
            case ButtonState::Normal: bgStyle = style["normal"]; break;
            case ButtonState::Hover:  bgStyle = style["hover"]; break;
            case ButtonState::Click:  bgStyle = style["click"]; break;
        }
        g.StyledRect(b.x, b.y, b.w, b.h, bgStyle);

        int cx = b.x + (int)padding.left;
        int cy = b.y + (int)padding.top;
        int contentH = b.h - (int)padding.GetVertical();

        // Checkmark
        if (m_checked) {
            Json checkmarkStyle = GetStyle()["CheckBox"]["checkmark"];
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
        g.StyledTextEnd(m_text, cx, cy + (contentH + (int)ex.height) / 2);

        // Submenu arrow
        if (m_subMenu) {
            Json arrowStyle = GetStyle()["MenuItem"]["arrow"];
            int arrowX = b.x + b.w - (int)padding.right - ICON_SIZE;
            int arrowY = b.y + (b.h - ICON_SIZE) / 2;
            g.DrawSVG(arrowStyle, arrowX, arrowY, ICON_SIZE, ICON_SIZE);
        }
    }

    void MenuItem::OnMouseDown(MouseEvent e) {
		if (m_separator) return;
		if (e.button != MouseButton::Left) return;

		if (m_state == ButtonState::Hover) {
			if (!m_subMenu) {
				m_state = ButtonState::Click;
				Invalidate();
			}
		}
    }

	void MenuItem::OnMouseUp(MouseEvent e) {
		if (m_separator) return;
		if (e.button != MouseButton::Left) return;

		if (m_state == ButtonState::Click) {
			Rectangle b = GetLocalBounds();
			if (b.HasPoint(e.x, e.y)) {
				if (m_onClick) m_onClick();
				m_state = ButtonState::Normal;
				if (auto* menu = GetParentMenu())
					menu->HideAll();
				Invalidate();
			}
		}
	}

	void MenuItem::OnMouseMove(MotionEvent e) {
		if (m_separator) return;
		Rectangle b = GetLocalBounds();

		if (b.HasPoint(e.x, e.y)) {
			if (m_state == ButtonState::Normal) {
				m_state = ButtonState::Hover;
				Invalidate();
				NotifyParentMenuHover();
			}
		} else {
			// Don't leave hover if mouse is in our open submenu
			bool inSubMenu = m_subMenu && m_subMenu->IsOpen()
				&& m_subMenu->HitTest(e.x, e.y);
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
        if (!menu) return;

        if (m_subMenu)
            menu->OpenSubMenu(this);
        else
            menu->CloseSubMenu();
    }

    Size MenuItem::GetPreferredSize() const {
        Json style = GetStyle()["MenuItem"];
        EdgeInsets padding = EdgeInsets::FromStyle(style["padding"]);

        if (IsAutoSize()) {
            if (m_separator) return { 0, 8 };

            auto& g = GetApp()->GetGraphics();
            const auto textStyle = GetStyle()["DefaultText"];

            g.Save();
            g.StyledTextBegin(textStyle);
            auto sz = g.MeasureText(m_text);
            g.Restore();

            int w = (int)padding.GetHorizontal() + CHECK_AREA + (int)sz.x_advance;
            int h = (int)padding.GetVertical() + (int)sz.height;

            if (m_icon) {
                w += ICON_SIZE + ICON_GAP;
                h = std::max(h, (int)padding.GetVertical() + ICON_SIZE);
            }
            if (m_subMenu) {
                w += SUBMENU_ARROW_AREA;
            }

            return { w, h };
        }
        return Element::GetPreferredSize();
    }

}