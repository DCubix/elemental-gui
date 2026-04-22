#pragma once

#include "Element.h"

namespace tui {
	class CheckBox : public Element {
	public:
		CheckBox();

		void OnMouseDown(MouseEvent e) override;
		void OnMouseUp(MouseEvent e) override;
		void OnMouseEnter() override;
		void OnMouseLeave() override;
		void OnDraw(Graphics& g) override;

		Size GetPreferredSize() const override;

		bool IsChecked() const { return m_checked; }
		void SetChecked(bool checked);

		std::string GetText() const { return m_text; }
		void SetText(const std::string& text) { m_text = text; Invalidate(); }

		void SetOnChanged(ValueChanged<bool> cb) { m_onChanged = cb; }

	private:
		std::string m_text;
		bool m_checked{ false };
		bool m_pressed{ false };

		ValueChanged<bool> m_onChanged;
	};
}
