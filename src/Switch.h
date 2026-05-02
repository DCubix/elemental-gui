#pragma once

#include "Element.h"

#include <functional>

namespace gui {
    class Switch : public Element {
    public:
        Switch();

        std::string StyleKey() const override { return "Switch"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnDraw(Graphics& g) override;

        Property<bool> checked{false};
        Property<bool> indeterminate{false};

    private:
        enum class State { Unchecked, Checked, Indeterminate } m_state{State::Unchecked};

        bool m_pressed{false};
    };
} // namespace gui