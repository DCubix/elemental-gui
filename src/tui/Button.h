#pragma once

#include "Label.h"

namespace tui {
	class Button : public Label {
	public:
		Button();

		void OnDraw(Graphics& g) override;
		EventStatus OnEvent(Event *event) override;
		
		Size GetPreferredSize() const override;

		void SetOnClick(const VoidCallback& cb) { m_onClick = cb; }

	private:
		VoidCallback m_onClick;
		ButtonState m_state;
	};
}
