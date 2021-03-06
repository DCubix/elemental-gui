#ifndef TUI_BUTTON_H
#define TUI_BUTTON_H

#include "Label.h"

namespace tui {
	using ClickCallback = std::function<void()>;

	class Button : public Label {
	public:
		Button();

		void onDraw(Graphics& g) override;
		EventStatus onEvent(Event *event) override;

		void onClick(ClickCallback cb) { m_onClick = cb; }

	private:
		enum ButtonState {
			BSNormal = 0,
			BSHover,
			BSClick
		};

		ClickCallback m_onClick;
		ButtonState m_state;
	};
}

#endif // TUI_BUTTON_H
