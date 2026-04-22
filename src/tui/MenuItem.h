#pragma once

#include "Element.h"
#include "Utils.h"

namespace tui {
    class Menu;
    class MenuItem : public Element {
    public:
        MenuItem(const std::string& text = "");

        void OnDraw(Graphics& g) override;

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseMove(MotionEvent e) override;

        Size GetPreferredSize() const override;

        std::string GetText() const { return m_text; }
        void SetText(const std::string& text) { m_text = text; Invalidate(); }

        Image* GetIcon() const { return m_icon; }
        void SetIcon(Image* icon) { m_icon = icon; Invalidate(); }

        void SetOnClick(VoidCallback callback) { m_onClick = callback; }

        bool IsChecked() const { return m_checked; }
        void SetChecked(bool checked) { m_checked = checked; Invalidate(); }

        bool IsSeparator() const { return m_separator; }
        void SetSeparator(bool separator) { m_separator = separator; Invalidate(); }

        Menu* GetSubMenu() const { return m_subMenu; }
        void SetSubMenu(Menu* subMenu) { m_subMenu = subMenu; Invalidate(); }

        Menu* GetParentMenu() const;

    private:
        std::string m_text;
        Image* m_icon{nullptr};
        Menu* m_subMenu{nullptr};
        VoidCallback m_onClick;
        ButtonState m_state{ButtonState::Normal};
        bool m_checked{false};
        bool m_separator{false};

        void NotifyParentMenuHover();
    };
}