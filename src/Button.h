#pragma once

#include "Label.h"

namespace gui {
    class Button : public Label {
      public:
        Button();

        void OnDraw(Graphics& g) override;

        std::string StyleKey() const override { return "Button"; }

        void OnMouseDown(MouseEvent e) override;
        void OnMouseUp(MouseEvent e) override;
        void OnMouseEnter() override;
        void OnMouseLeave() override;

        Size GetPreferredSize() const override;

        void SetOnClick(const VoidCallback& cb) { m_onClick = cb; }

      private:
        VoidCallback m_onClick;
        ButtonState m_state;
    };
} // namespace gui
