#include "EventSystem.h"

namespace gui {

	void EventSystem::Subscribe(Subscriber* sub, EventType event) {
		m_subscribers[event].push_back(sub);
	}

}
