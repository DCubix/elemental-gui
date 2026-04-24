#pragma once

#include "Element.h"

namespace tui {
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

        std::string GetLabel() const { return m_label; }
        void SetLabel(const std::string& label) { m_label = label; Invalidate(); }

        void SetOnChanged(ValueChanged<bool> cb) { m_onChanged = cb; }

    private:
        bool m_on{ false };
        bool m_pressed{ false };
        std::string m_label;
        ValueChanged<bool> m_onChanged;
    };
}
