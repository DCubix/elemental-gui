#pragma once

#include "Label.h"

namespace gui {
    class ToolButton : public Label {
    public:
        enum class Mode { Normal = 0, Toggle, Radio };

        ToolButton();

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "ToolButton"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;

        Size GetPreferredSize() const override;

        Mode GetMode() const { return m_mode; }
        void SetMode(Mode mode) { m_mode = mode; }

        std::string GetGroup() const { return m_group; }
        void SetGroup(const std::string& group) { m_group = group; }

        bool IsToggled() const { return m_toggled; }
        void SetToggled(bool toggled) { m_toggled = toggled; }

        void SetOnClick(const VoidCallback& cb) { m_onClick = cb; }

    private:
        VoidCallback m_onClick;
        ButtonState m_state;
        Mode m_mode;
        std::string m_group{""}; // For radio buttons
        bool m_toggled{false};   // For toggle and radio buttons
    };
} // namespace gui