#include "Edit.h"

#include "Window.h"
#include "Layout.h"

namespace gui
{

    Edit::Edit() {
        SetLocalBounds(Rectangle(0, 0, 120, 28));
    }

    void Edit::OnCreate() {
        SetText("");
    }

    void Edit::OnDraw(Graphics& g) {
        auto sz = GetSize();

        std::string state = "normal";
        if (IsFocused())    state = "focused";
        else if (m_hovered) state = "hover";

        g.StyledRect(0, 0, sz.w, sz.h, GetStyle()[state]);
        LineEdit::OnDraw(g);
    }

    void Edit::OnMouseEnter() {
        Invalidate();
    }

    void Edit::OnMouseLeave() {
        Invalidate();
    }

    void Edit::OnFocus(FocusEvent e) {
        LineEdit::OnFocus(e);
        Invalidate();
    }

    void Edit::OnBlur(BlurEvent e) {
        LineEdit::OnBlur(e);
        Invalidate();
    }

}
