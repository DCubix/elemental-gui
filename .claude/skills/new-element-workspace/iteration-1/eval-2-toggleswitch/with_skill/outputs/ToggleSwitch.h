#pragma once

#include "Element.h"

namespace gui {
    class ToggleSwitch : public Element {
    public:
        ToggleSwitch();

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;
        void OnDraw(Graphics& g) override;

        Size GetPreferredSize() const override;

        bool IsOn() const { return m_on; }
        void SetOn(bool on);

        void SetOnChanged(ValueChanged<bool> cb) { m_onChanged = cb; }

    private:
        enum class State { Normal, Hover, Click };
        State m_state{State::Normal};

        bool m_on{false};

        ValueChanged<bool> m_onChanged;
    };
}
