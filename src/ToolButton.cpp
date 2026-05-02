#include "ToolButton.h"

#include "Layout.h"
#include "Window.h"

namespace gui {
    ToolButton::ToolButton()
        : Label() {
        SetLocalBounds(Rectangle(0, 0, 50, 22));
        alignment = Alignment::MiddleCenter;
        text = "Button";
        m_state = ButtonState::Normal;
        toggled.SetOnUpdate([this]{ Invalidate(); });
    }

    void ToolButton::OnDraw(Graphics& g) {
        auto sz = GetSize();

        std::string state = "";
        switch (m_state) {
            case ButtonState::Normal:
                state = "";
                break;
            case ButtonState::Hover:
                state = "hover";
                break;
            case ButtonState::Click:
                state = "click";
                break;
        }

        if (toggled()) {
            state = "click";
        }

        if (state != "") {
            g.StyledRect(0, 0, sz.w, sz.h, GetStyle()[state]);
        }
        Label::OnDraw(g);
    }

    void ToolButton::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Click;
            if (m_mode == Mode::Toggle) {
                toggled = !toggled();
            } else if (m_mode == Mode::Radio) {
                toggled = true;
            }
            Invalidate();
        }
    }

    void ToolButton::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (m_state == ButtonState::Click) {
            if (m_onClick)
                m_onClick();
            if (m_mode == Mode::Radio) {
                auto* prev = m_window->Find<ToolButton>([this](ToolButton* tb) {
                    return tb != this && tb->GetGroup() == GetGroup() &&
                           tb->GetMode() == Mode::Radio && tb->toggled();
                });
                if (prev) {
                    prev->toggled = false;
                }
            }
            m_state = ButtonState::Hover;
            Invalidate();
        }
    }

    void ToolButton::OnMouseEnter() {
        if (m_state == ButtonState::Normal) {
            m_state = ButtonState::Hover;
            Invalidate();
        }
    }

    void ToolButton::OnMouseLeave() {
        if (m_state == ButtonState::Hover) {
            m_state = ButtonState::Normal;
            Invalidate();
        } else if (m_state == ButtonState::Click) {
            m_state = ButtonState::Normal;
            Invalidate();
        }
    }

    Size ToolButton::GetPreferredSize() const {
        if (IsAutoSize()) {
            Size size = Label::GetPreferredSize();
            EdgeInsets padding = EdgeInsets::FromStyle(GetStyle()["padding"]);
            size.w += padding.left + padding.right;
            size.h += padding.top + padding.bottom;
            return size;
        }
        return Element::GetPreferredSize();
    }
} // namespace gui
