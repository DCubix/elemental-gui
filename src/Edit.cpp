#include "Edit.h"

#include "Layout.h"
#include "Window.h"

namespace gui {

    Edit::Edit()
        : LineEdit() {
        SetLocalBounds(Rectangle(0, 0, 120, 28));
    }

    void Edit::OnCreate() {
        SetText("");
    }

    void Edit::OnDraw(Graphics& g) {
        auto sz = GetSize();

        std::string state = "normal";
        if (IsFocused())
            state = "focused";
        else if (m_hovered)
            state = "hover";

        auto style = GetStyle()[state];
        g.StyledRect(0, 0, sz.w, sz.h, style);
        LineEdit::OnDraw(g);
    }

    void Edit::OnMouseEnter() {
        Invalidate();
    }

    void Edit::OnMouseLeave() {
        Invalidate();
    }

} // namespace gui
