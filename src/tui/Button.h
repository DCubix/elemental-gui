#ifndef TUI_BUTTON_H
#define TUI_BUTTON_H

#include "Label.h"

namespace tui {
	using ClickCallback = std::function<void()>;

	class Button : public Label {
	public:
		Button();

		void OnDraw(Graphics& g) override;
		EventStatus OnEvent(Event *event) override;

		void SetOnClick(ClickCallback cb) { m_onClick = cb; }

	private:
		enum class ButtonState {
			Normal = 0,
			Hover,
			Click
		};

		ClickCallback m_onClick;
		ButtonState m_state;
	};
}

#endif // TUI_BUTTON_H
