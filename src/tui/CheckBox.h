#pragma once

#include "Element.h"

#include <functional>

namespace tui {
	class CheckBox : public Element {
	public:
		CheckBox();

		EventStatus OnEvent(Event *event) override;
		void OnDraw(Graphics& g) override;

		Size GetPreferredSize() const override;

		bool IsChecked() const { return m_checked; }
		void SetChecked(bool checked);

		std::string GetText() const { return m_text; }
		void SetText(const std::string& text) { m_text = text; Invalidate(); }

		void SetOnChanged(std::function<void(bool)> cb) { m_onChanged = cb; }

	private:
		std::string m_text;
		bool m_checked{ false };
		bool m_hovered{ false };
		bool m_pressed{ false };

		std::function<void(bool)> m_onChanged;
	};
}
