#ifndef TUI_BUTTON_H
#define TUI_BUTTON_H

#include "Label.h"

namespace tui {
	using Callback = std::function<void()>;

	class Button : public Label {
	public:
		Button();

		void onDraw(Graphics& g) override;
		EventStatus onEvent(Event *event) override;

		void onClick(Callback cb) { m_onClick = cb; }

	private:
		enum ButtonState {
			BSNormal = 0,
			BSHover,
			BSClick
		};

		Callback m_onClick;
		ButtonState m_state;
	};
}

#endif // TUI_BUTTON_H
