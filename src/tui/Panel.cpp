#include "Panel.h"
#include "FlexLayout.h"

#include "Application.h"

namespace tui {

	Panel::Panel() : Container(), m_backgroundVisible(true) {
		m_layout = std::make_unique<FlexLayout>();
		m_layout->SetGap(4);
		m_layout->SetPadding(EdgeInsets::All(6));
	}

	void Panel::Add(Element *element) {
		Container::Add(element);
		if (FlexLayout* flex = dynamic_cast<FlexLayout*>(m_layout.get())) {
			flex->Add(element);
		}
	}

    void Panel::Remove(Element *element)
    {
        Container::Remove(element);
        if (FlexLayout* flex = dynamic_cast<FlexLayout*>(m_layout.get())) {
            flex->Remove(element);
        }
    }

    void Panel::SetLayout(std::unique_ptr<Layout> layout) {
		m_layout = std::move(layout);
	}

	void Panel::OnDraw(Graphics& g) {
		Rectangle b = GetBounds();
		if (m_backgroundVisible)
			g.StyledRect(b.x, b.y, b.w, b.h, GetStyle()["Panel"]);

		if (m_layout)
			m_layout->Apply(b);

		EdgeInsets pad = m_layout ? m_layout->GetPadding() : EdgeInsets();
		Rectangle c = GetIntersectedBounds();
		Rectangle padded{
			c.x + pad.left,
			c.y + pad.top,
			c.w - pad.GetHorizontal(),
			c.h - pad.GetVertical()
		};
		g.ClipPushRect(padded.x, padded.y, padded.w, padded.h);
		Container::OnDraw(g);
		g.ClipPop();
	}

	Size Panel::GetPreferredSize() const {
		if (!IsAutoSize()) return Element::GetPreferredSize();
		return m_layout->GetLaidOutSize();
	}

}
