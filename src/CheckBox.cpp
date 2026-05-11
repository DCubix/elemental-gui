#include "CheckBox.h"

#include "Application.h"

namespace gui {

    CheckBox::CheckBox()
        : Element() {
        SetLocalBounds(Rectangle(0, 0, 100, 22));
        TrackAll(checked, text);
        checked.Bind([this](const auto&) { NotifyListeners(); });
    }

    void CheckBox::OnMouseDown(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        m_pressed = true;
        Invalidate();
    }

    void CheckBox::OnMouseUp(MouseEvent e) {
        if (e.button != MouseButton::Left)
            return;
        if (m_pressed) {
            m_pressed = false;
            checked = !checked();
        }
    }

    void CheckBox::OnMouseEnter() {
        Invalidate();
    }

    void CheckBox::OnMouseLeave() {
        Invalidate();
    }

    void CheckBox::OnDraw(Graphics& g) {
        Size size = GetSize();
        Json style = GetStyle();

        int boxSize = style.value("size", size.h);
        int boxX = 0;
        int boxY = (size.h - boxSize) / 2;

        // Select state-based style
        std::string boxState = "normal";
        if (checked()) {
            boxState = (m_hovered && !m_pressed) ? "checkedHover" : "checked";
        } else if (m_pressed) {
            boxState = "click";
        } else if (m_hovered) {
            boxState = "hover";
        }

        // Draw the checkbox box
        g.StyledRect(boxX, boxY, boxSize, boxSize, style[boxState]);

        // Draw checkmark when checked
        if (checked() && style["checkmark"].is_object()) {
            g.DrawSVG(style["checkmark"], boxX, boxY, boxSize, boxSize);
        }

        // Draw label text
        if (!text().empty()) {
            int textX = boxX + boxSize + 6;
            int textY = boxY;

            g.StyledTextBegin(style);
            auto ex = g.MeasureText(text());
            int textOffY = boxSize / 2 + static_cast<int>(ex.size.h) / 2;
            g.StyledTextEnd(text(), textX, textY + textOffY);
        }
    }

    Size CheckBox::GetPreferredSize() const {
        Json style = GetStyle();
        int boxSize = style.value("size", m_bounds.h);
        return {m_bounds.w, boxSize};
    }
} // namespace gui
