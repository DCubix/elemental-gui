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

        bool IsChecked() const { return m_state == State::Checked; }
        void SetChecked(bool checked);

        bool IsIndeterminate() const { return m_state == State::Indeterminate; }
        void SetIndeterminate(bool indeterminate);

        void SetOnChanged(std::function<void(bool)> cb) { m_onChanged = cb; }

    private:
        enum class State { Unchecked, Checked, Indeterminate } m_state{State::Unchecked};

        bool m_pressed{false};

        std::function<void(bool)> m_onChanged;
    };
} // namespace gui