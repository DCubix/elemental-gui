#ifndef TUI_EVENT_SYSTEM_H
#define TUI_EVENT_SYSTEM_H

#include <map>
#include <vector>
#include <type_traits>

namespace tui {
	class Subscriber;

	struct Event {
		virtual int type() { return -1; }
	};

	// Default events
	enum EventType {
		MouseEventType = 0,
		MotionEventType,
		KeyEventType,
		TextInputEventType,
		FocusEventType,
		BlurEventType
	};

	struct MouseEvent : public Event {
		int type() { return EventType::MouseEventType; }

		MouseEvent() = default;
		MouseEvent(int x, int y, int button, bool pressed)
			: x(x), y(y), button(button), pressed(pressed)
		{}

		int x, y, button;
		bool pressed;
	};

	struct MotionEvent : public Event {
		int type() { return EventType::MotionEventType; }

		MotionEvent() = default;
		MotionEvent(int x, int y, int button)
			: x(x), y(y), button(button)
		{}

		int x, y, button;
	};

	struct TextInput : public Event {
		int type() { return EventType::TextInputEventType; }

		TextInput() = default;
		TextInput(char c) : inputChar(c) {}

		char inputChar;
	};

	struct FocusEvent : public Event {
		int type() { return EventType::FocusEventType; }

		FocusEvent() = default;
		FocusEvent(Subscriber *e) : element(e) {}

		Subscriber *element;
	};

	struct BlurEvent : public Event {
		int type() { return EventType::BlurEventType; }

		BlurEvent() = default;
		BlurEvent(Subscriber *e) : element(e) {}

		Subscriber *element;
	};

	struct KeyEvent : public Event {
		int type() { return EventType::KeyEventType; }

		KeyEvent() = default;
		KeyEvent(int key, int mod, bool pressed)
			: key(key), mod(mod), pressed(pressed)
		{}

		int key, mod;
		bool pressed;
	};

	//
	enum EventStatus {
		Consumed = 0,
		Active
	};
	class Subscriber {
	public:
		virtual EventStatus onEvent(Event *event) = 0;

		bool enabled() const { return m_enabled; }
		void enabled(bool v) { m_enabled = v; }

	private:
		bool m_enabled = true;
	};
	using EventSubscriberMap = std::map<int, std::vector<Subscriber*>>;

	class EventSystem {
	public:
		EventSystem() = default;
		~EventSystem() = default;

		void subscribe(Subscriber *sub, int event);

		template <typename E, typename... Args>
		void broadcast(Args&&... args) {
			static_assert(std::is_base_of<Event, E>::value, "E must inherit from Event");

			E *e = new E(args...);

			auto evt = m_subscribers.find(e->type());
			if (evt == m_subscribers.end()) return;

			for (auto&& sub : evt->second) {
				if (!sub->enabled()) continue;

				EventStatus status = sub->onEvent(e);
				if (status == EventStatus::Consumed) {
					break;
				}
			}

			delete e;
		}

	private:
		EventSubscriberMap m_subscribers;
	};
}

#endif // TUI_EVENT_SYSTEM_H
