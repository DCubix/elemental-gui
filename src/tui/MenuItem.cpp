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
        : Element(), m_text(text)
    {
        SetLocalBounds({0, 0, 100, 20});
        SetAutoSize(true);
    }
    
    void MenuItem::OnDraw(Graphics &g)
    {
        if (m_separator) return; // Separators are drawn by Menu

        Json style = GetStyle()["MenuItem"];
        EdgeInsets padding = EdgeInsets::FromStyle(style["padding"]);
        Json textStyle = GetStyle()["DefaultText"];

        Rectangle b = GetBounds();

        // Background
        Json bgStyle;
        switch (m_state) {
            case ButtonState::Normal: bgStyle = style["normal"]; break;
            case ButtonState::Hover: bgStyle = style["hover"]; break;
            case ButtonState::Click: bgStyle = style["click"]; break;
        }
        g.StyledRect(b.x, b.y, b.w, b.h, bgStyle);

        int cx = b.x + (int)padding.left;
        int cy = b.y + (int)padding.top;

        // Checkmark
        if (m_checked) {
            Json checkmarkStyle = GetStyle()["CheckBox"]["checkmark"];
            g.DrawSVG(checkmarkStyle, cx + 2, cy, ICON_SIZE, ICON_SIZE);
        }
        cx += CHECK_AREA;

        // Icon
        if (m_icon) {
            int iconY = cy + (b.h - (int)padding.GetVertical() - ICON_SIZE) / 2;
            g.DrawImage(m_icon, cx, iconY, ICON_SIZE, ICON_SIZE);
            cx += ICON_SIZE + ICON_GAP;
        }

        // Text
        g.StyledTextBegin(textStyle);
        auto ex = g.MeasureText(m_text);
        int textY = cy + (b.h - (int)padding.GetVertical() + (int)ex.height) / 2;
        g.StyledTextEnd(m_text, cx, textY);

        // Submenu arrow indicator
        if (m_subMenu) {
            Json arrowStyle = GetStyle()["MenuItem"]["arrow"];
            int arrowSize = ICON_SIZE;
            int arrowX = b.x + b.w - (int)padding.right - arrowSize;
            int arrowY = b.y + (b.h - arrowSize) / 2;
            g.DrawSVG(arrowStyle, arrowX, arrowY, arrowSize, arrowSize);
        }
    }
    
    EventStatus MenuItem::OnEvent(Event *event)
    {
        if (m_separator) return EventStatus::Active;

        EventStatus status = Element::OnEvent(event);
		if (event->Type() == EventType::MouseButton) {
			Rectangle b = GetIntersectedBounds();
			MouseEvent* e = dynamic_cast<MouseEvent*>(event);
			switch (m_state) {
				case ButtonState::Hover: {
					if (e->pressed && e->button == 1) {
						// If this item has a submenu, don't go to click state
						if (m_subMenu) {
							status = EventStatus::Consumed;
						} else {
							m_state = ButtonState::Click;
							status = EventStatus::Consumed;
							Invalidate();
						}
					}
				} break;
				case ButtonState::Click: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Normal;
						status = EventStatus::Consumed;
						Invalidate();
					}
					if (!e->pressed && e->button == 1) {
						if (m_onClick)
							m_onClick();
						m_state = ButtonState::Normal;
						// Dismiss the entire menu chain — walk up to root menu
						if (auto* menu = dynamic_cast<Menu*>(GetParent())) {
							Menu* root = menu;
							while (root->GetParentMenu()) {
								root = root->GetParentMenu();
							}
							root->HideAll();
						}
						status = EventStatus::Consumed;
						Invalidate();
					}
				} break;
				default: break;
			}
		} else if (event->Type() == EventType::MouseMotion) {
			Rectangle b = GetIntersectedBounds();
			MotionEvent* e = dynamic_cast<MotionEvent*>(event);
			switch (m_state) {
				case ButtonState::Normal: {
					if (b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Hover;
						Invalidate();
						status = EventStatus::Consumed;
						// Open submenu on hover
						if (m_subMenu) {
							if (auto* menu = dynamic_cast<Menu*>(GetParent())) {
								menu->OpenSubMenu(this);
							}
						} else {
							// Close any open submenu when hovering a non-submenu item
							if (auto* menu = dynamic_cast<Menu*>(GetParent())) {
								menu->CloseSubMenu();
							}
						}
					}
				} break;
				case ButtonState::Hover: {
					if (!b.HasPoint(e->x, e->y)) {
						// Don't leave hover if mouse moved into our open submenu
						bool inSubMenu = false;
						if (m_subMenu && m_subMenu->IsOpen()) {
							inSubMenu = m_subMenu->GetBounds().HasPoint(e->x, e->y);
						}
						if (!inSubMenu) {
							m_state = ButtonState::Normal;
							Invalidate();
						}
					} else {
						status = EventStatus::Consumed;
					}
				} break;
				case ButtonState::Click: {
					if (!b.HasPoint(e->x, e->y)) {
						m_state = ButtonState::Normal;
						Invalidate();
					} else {
						status = EventStatus::Consumed;
					}
				} break;
			}
		}
		return status;
    }
    
    Size MenuItem::GetPreferredSize() const
    {
        Json style = GetStyle()["MenuItem"];
        EdgeInsets padding = EdgeInsets::FromStyle(style["padding"]);

        if (IsAutoSize()) {
            if (m_separator) {
                return { 0, 8 };
            }

            const auto textStyle = GetStyle()["DefaultText"];
            auto& g = GetApp()->GetGraphics();

            g.Save();
            g.StyledTextBegin(textStyle);

            auto sz = g.MeasureText(m_text);
            int textW = (int)sz.x_advance;
            int textH = (int)sz.height;

            int w = (int)padding.GetHorizontal() + CHECK_AREA + textW;
            int h = (int)padding.GetVertical() + textH;

            if (m_icon) {
                w += ICON_SIZE + ICON_GAP;
                h = std::max(h, (int)padding.GetVertical() + ICON_SIZE);
            }

            if (m_subMenu) {
                w += SUBMENU_ARROW_AREA;
            }

            g.Restore();

            return { w, h };
        }
        return Element::GetPreferredSize();
    }
}