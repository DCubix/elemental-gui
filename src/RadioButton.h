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

        std::string GetGroup() const { return m_group; }
        void SetGroup(const std::string& group) { m_group = group; }

        std::string GetText() const { return m_text; }
        void SetText(const std::string& text) { m_text = text; Invalidate(); }

        bool IsChecked() const { return m_checked; }
        void SetChecked(bool checked);

        void SetOnChanged(ValueChanged<bool> cb) { m_onChanged = cb; }
    private:
        std::string m_group{""}, m_text;
        bool m_checked{false};
        bool m_pressed{ false };

		ValueChanged<bool> m_onChanged;
    };
}