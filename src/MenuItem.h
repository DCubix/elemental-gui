#pragma once

#include "Element.h"
#include "Utils.h"

namespace gui {
    class Menu;
    class MenuItem : public Element {
    public:
        MenuItem(const std::string& text = "");

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "MenuItem"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;

        void ClearHoverState();

        Size GetPreferredSize() const override;

        Image* GetIcon() const { return m_icon; }
        void SetIcon(Image* icon) {
            m_icon = icon;
            Invalidate();
        }

        void SetOnClick(VoidCallback callback) { m_onClick = callback; }

        bool IsSeparator() const { return m_separator; }
        void SetSeparator(bool separator) {
            m_separator = separator;
            Invalidate();
        }

        Menu* GetSubMenu() const { return m_subMenu; }
        void SetSubMenu(Menu* subMenu) {
            m_subMenu = subMenu;
            Invalidate();
        }

        Menu* GetParentMenu() const;

        Property<std::string> text;
        Property<bool> checked{false};

    private:
        Image* m_icon{nullptr};
        Menu* m_subMenu{nullptr};
        VoidCallback m_onClick;
        ButtonState m_state{ButtonState::Normal};
        bool m_checked{false};
        bool m_separator{false};

        void NotifyParentMenuHover();
    };
} // namespace gui