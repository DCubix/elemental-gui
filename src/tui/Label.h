#pragma once

#include "Element.h"

namespace tui {

	class Label : public Element {
	public:
		Label();

		void OnDraw(Graphics& g) override;

		Size GetPreferredSize() const override;

		std::string GetText() const { return m_text; }
		void SetText(const std::string& text) { m_text = text; Invalidate(); }

		Alignment GetAlignment() const { return m_alignment; }
		void SetAlignment(Alignment align) { m_alignment = align; Invalidate(); }

		Image* GetIcon() { return m_icon; }
		void SetIcon(Image* image) { m_icon = image; Invalidate(); }

	protected:
		std::string m_text;
		Alignment m_alignment;
		Image* m_icon;
	};
}
