#pragma once

#include "MenuItem.h"

namespace gui {
    class Application;
    class Menu : public Element {
    public:
        Menu();
        ~Menu() = default;

        void OnDraw(Graphics& g) override;
        std::string StyleKey() const override { return "Menu"; }
        EventStatus OnEvent(Event *event) override;

		Size GetPreferredSize() const override;

        void Add(MenuItem* item);

        void Show(int x, int y);
        void Hide();
        void HideAll();
        bool IsOpen() const { return m_open; }
        bool HitTest(int x, int y) const;
        bool HasOpenSubMenu() const;

        void SetOnDismiss(VoidCallback callback) { m_onDismiss = callback; }

        void OpenSubMenu(MenuItem* item);
        void CloseSubMenu();

        Menu* GetParentMenu() const { return m_parentMenu; }
        void SetParentMenu(Menu* parent) { m_parentMenu = parent; }

    private:
        std::vector<MenuItem*> m_items;
        bool m_open{false};
        VoidCallback m_onDismiss;
        MenuItem* m_activeSubMenuItem{nullptr};
        Menu* m_parentMenu{nullptr};

        void PlaceItems();
    };

}