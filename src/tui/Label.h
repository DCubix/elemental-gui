#ifndef TUI_LABEL_H
#define TUI_LABEL_H

#include "Element.h"

namespace tui {
	enum Alignment {
		TopLeft = 0,
		TopCenter,
		TopRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		BottomLeft,
		BottomCenter,
		BottomRight,
		AlignmentCount
	};

	class Label : public Element {
	public:
		Label();

		void onDraw(Graphics& g) override;

		std::string text() const { return m_text; }
		void text(const std::string& text) { m_text = text; invalidate(); }

		Alignment alignment() const { return m_alignment; }
		void alignment(Alignment align) { m_alignment = align; invalidate(); }

		Json style() const { return m_style; }
		void style(Json style) { m_style = style; invalidate(); }

		Image* icon() { return m_icon; }
		void icon(Image* image) { m_icon = image; invalidate(); }

	protected:
		Json m_style;
		std::string m_text;
		Alignment m_alignment;

		Image* m_icon;
	};
}

#endif // TUI_LABEL_H
