#include "EventSystem.h"

#include <algorithm>

namespace gui {

    void EventSystem::Subscribe(Subscriber* sub, EventType event) {
        m_subscribers[event].push_back(sub);
    }

    void EventSystem::UnsubscribeAll(Subscriber* sub) {
        for (auto& [type, subs] : m_subscribers)
            subs.erase(std::remove(subs.begin(), subs.end(), sub), subs.end());
    }

} // namespace gui
