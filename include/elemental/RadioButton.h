#pragma once

#include "Element.h"

namespace gui {
    class RadioButton : public Element {
    public:
        RadioButton();

        std::string StyleKey() const override { return "RadioButton"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnDraw(Graphics& g) override;

        Size GetPreferredSize() const override;

        Property<bool> checked{false};
        Property<std::string> text;
        Property<std::string> group;

    private:
        bool m_pressed{false};
    };
} // namespace gui