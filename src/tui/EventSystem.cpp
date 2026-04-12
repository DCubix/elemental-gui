#include "EventSystem.h"

namespace tui {

	void EventSystem::Subscribe(Subscriber* sub, EventType event) {
		m_subscribers[event].push_back(sub);
	}

}
