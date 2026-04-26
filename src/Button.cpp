#include "Button.h"

#include "EventSystem.h"
#include "Layout.h"

namespace gui
{

    Button::Button() : Label() {
        SetLocalBounds(Rectangle(0, 0, 50, 22));
        m_alignment = Alignment::MiddleCenter;
        m_text = "Button";
        m_state = ButtonState::Normal;
    }

    void Button::OnDraw(Graphics& g) {
        auto sz = GetSize();

        EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);

        std::string state = "normal";
        switch (m_state) {
            case ButtonState::Normal: state = "normal"; break;
            case ButtonState::Hover: state = "hover"; break;
            case ButtonState::Click: state = "click"; break;
        }

        g.StyledRect(0, 0, sz.w, sz.h, GetStyle()[state]);
        Label::OnDraw(g);
    }

    void Button::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Click;
            Invalidate();
        }
    }

    void Button::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left) return;
        if (m_state == ButtonState::Click) {
            if (m_onClick)
                m_onClick();
            m_state = ButtonState::Hover;
            Invalidate();
        }
    }

    void Button::OnMouseEnter() {
        if (m_state == ButtonState::Normal) {
            m_state = ButtonState::Hover;
            Invalidate();
        }
    }

    void Button::OnMouseLeave() {
        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Normal;
            Invalidate();
        }
        else if (m_state == ButtonState::Click) {
            m_state = ButtonState::Normal;
            Invalidate();
        }
    }

    Size Button::GetPreferredSize() const {
        if (IsAutoSize()) {
            Size size = Label::GetPreferredSize();
            EdgeInsets padding = EdgeInsets::FromStyle(GetStyle()["padding"]);
            size.w += padding.left + padding.right;
            size.h += padding.top + padding.bottom;
            return size;
        }
        return Element::GetPreferredSize();
    }
}
