#ifndef TUI_LABEL_H
#define TUI_LABEL_H

#include "Element.h"

namespace tui {
	enum class Alignment {
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

		void OnDraw(Graphics& g) override;

		std::string GetText() const { return m_text; }
		void SetText(const std::string& text) { m_text = text; Invalidate(); }

		Alignment GetAlignment() const { return m_alignment; }
		void SetAlignment(Alignment align) { m_alignment = align; Invalidate(); }

		Json GetStyle() const { return m_style; }
		void SetStyle(Json style) { m_style = style; Invalidate(); }

		Image* GetIcon() { return m_icon; }
		void SetIcon(Image* image) { m_icon = image; Invalidate(); }

	protected:
		Json m_style;
		std::string m_text;
		Alignment m_alignment;

		Image* m_icon;
	};
}

#endif // TUI_LABEL_H
