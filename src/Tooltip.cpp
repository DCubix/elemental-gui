#include "Tooltip.h"

#include "Layout.h"

namespace gui {
    Tooltip::Tooltip()
        : Label() {
        alignment = utils::Alignment::MiddleCenter;
    }

    void Tooltip::OnDraw(Graphics& g) {
        auto sz = GetSize();
        auto style = GetStyle();
        g.StyledRect(0, 0, sz.w, sz.h, style);
        m_textStyle = style;
        Label::OnDraw(g);
    }

    Size Tooltip::GetPreferredSize() const {
        Size textSize = Label::GetPreferredSize();
        EdgeInsets pad = EdgeInsets::FromStyle(GetStyle()["padding"]);
        return {
            textSize.w + (int)pad.left + (int)pad.right,
            textSize.h + (int)pad.top + (int)pad.bottom
        };
    }

} // namespace gui
