#include "EventSystem.h"

namespace tui {

	void EventSystem::subscribe(Subscriber* sub, int event) {
		m_subscribers[event].push_back(sub);
	}

}
