#pragma once

#include "Element.h"

namespace gui {
    class CheckBox : public Element {
    public:
        CheckBox();

        std::string StyleKey() const override { return "CheckBox"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnDraw(Graphics& g) override;

        Size GetPreferredSize() const override;

        Property<bool> checked{false};
        Property<std::string> text;

    private:
        bool m_pressed{false};
    };
} // namespace gui
