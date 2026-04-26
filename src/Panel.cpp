#include "Panel.h"

#include "Application.h"
#include "FlexLayout.h"

namespace gui {

    Panel::Panel()
        : Container(),
          m_backgroundVisible(true) {
        m_layout = std::make_unique<FlexLayout>();
        m_layout->SetGap(4);
        m_layout->SetPadding(EdgeInsets::All(6));
    }

    void Panel::Add(Element* element) {
        Container::Add(element);
        if (FlexLayout* flex = dynamic_cast<FlexLayout*>(m_layout.get())) {
            flex->Add(element);
        }
    }

    void Panel::Remove(Element* element) {
        Container::Remove(element);
        if (FlexLayout* flex = dynamic_cast<FlexLayout*>(m_layout.get())) {
            flex->Remove(element);
        }
    }

    void Panel::SetLayout(std::unique_ptr<Layout> layout) {
        m_layout = std::move(layout);
    }

    void Panel::OnDraw(Graphics& g) {
        Size size = GetSize();
        if (m_backgroundVisible)
            g.StyledRect(0, 0, size.w, size.h, GetStyle());

        if (m_layout)
            m_layout->Apply({0, 0, size.w, size.h});

        EdgeInsets pad = m_layout ? m_layout->GetPadding() : EdgeInsets();
        Rectangle clip = GetLocalIntersectedBounds();
        Rectangle padded{
            static_cast<int>(clip.x + pad.left),
            static_cast<int>(clip.y + pad.top),
            static_cast<int>(clip.w - pad.GetHorizontal()),
            static_cast<int>(clip.h - pad.GetVertical())
        };
        g.ClipPushRect(padded.x, padded.y, padded.w, padded.h);
        Container::OnDraw(g);
        g.ClipPop();
    }

    Size Panel::GetPreferredSize() const {
        if (!IsAutoSize())
            return Element::GetPreferredSize();
        return m_layout->GetLaidOutSize();
    }

} // namespace gui
